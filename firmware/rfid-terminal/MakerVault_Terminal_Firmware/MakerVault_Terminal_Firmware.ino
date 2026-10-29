#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_NeoPixel.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include "firmware_version.h"
#include "brand_images.h"
#include "ui_icons.h"
#include "provisioning.h"
#include "connected_registration.h"
#include "scan_lookup.h"
#include "remote_lookup.h"
#include "ntag215_feature_gate.h"
// The experimental writer is excluded at compile time until device acceptance.
#if MAKERVAULT_NDEF_WRITER_EXPERIMENTAL
#include "connected_ndef_write.h"
#endif

// MakerVault handheld UI v0.6.1 -- ESP32-S3 SuperMini, Rev C.3
// Shared hardware SPI, read-only RFID, all original GPIO assignments.
constexpr uint8_t SCK_PIN=12, MOSI_PIN=11, MISO_PIN=13;
constexpr uint8_t TFT_CS=1, TFT_DC=2, TFT_RST=43;
constexpr uint8_t RFID_CS=10, RFID_RST=9;
constexpr uint8_t ENC_A=4, ENC_B=5, ENC_SW=6;
constexpr uint8_t BACK_PIN=7, CONFIRM_PIN=8, RGB_PIN=48;
Adafruit_ST7735 tft(&SPI,TFT_CS,TFT_DC,TFT_RST);
Adafruit_GFX *display = &tft; // Hardware TFT; no temporary screenshot buffer
MFRC522 rfid(RFID_CS,RFID_RST);
Adafruit_NeoPixel led(1,RGB_PIN,NEO_GRB+NEO_KHZ800);

// RGB565 palette, crafted for a small TFT.
constexpr uint16_t BG=0x0843, CARD=0x10C6, NAVY=0x10ED,
  BLUE=0x159F, SKY=0x867F, WHITE=0xFFFF, SILVER=0xCE79,
  MUTED=0x94D2, GREEN=0x464C, AMBER=0xF5A2, LINE=0x296A;

// Write actions remain feature gated until fully implemented. API scope alone
// never authorises a placeholder to appear as a working device operation.
constexpr bool MAKERTAG_CREATE_IMPLEMENTED=false, RFID_WRITE_IMPLEMENTED=false;
enum Page:uint8_t{HOME,SCAN,RECENT,CONTROLS,INFO,CONNECTIONS,SETUP_WIFI,KNOWN_WIFI,DELETE_WIFI,MAKERVAULT_SETUP,MV_STATUS,MV_PAIR,MAKERTAGS_MENU,RFID_MENU,UNAVAILABLE,WIFI_MENU,MV_MENU,MV_FORGET,WIFI_STATUS,MV_TLS_TEST,MV_CA_REMOVE,REGISTRATION,LOOKUP_REQUEST,NDEF_WRITE};
Page page=HOME;
const char* choices[]={"MakerTags","RFID Tags","Connections","System info"};
int subSelected=0, detailScroll=0, detailCount=0;
String unavailableTitle="";
int selected=0;
int connSelected=0,networkSelected=0;
int wifiSelected=0,mvSelected=0;
uint8_t shownWifiState=255,shownMvState=255;uint32_t footerChecked=0;
String pendingDeleteSSID="";
// Persist manual offline mode independently of remembered SSIDs.

bool scanResultVisible=false, hasScanned=false, knownReader=false;
uint8_t versionReg=0;
String recentUid="", recentType="";
uint32_t lastPoll=0,feedbackUntil=0;
bool readerInitDone=false; bool readerBusVerified=false; uint32_t bootAt=0;
uint32_t lastInputAt=0;
const int8_t decode[16]={0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
uint8_t previousAB=0;
int8_t stepCount=0;
uint32_t lastEncoderTurnAt=0;

struct Key {
 uint8_t pin; bool raw=true,stable=true;uint32_t changed=0;
 void begin(uint8_t p){pin=p;pinMode(pin,INPUT_PULLUP);raw=stable=digitalRead(pin);changed=millis();}
 bool pressed(){bool next=digitalRead(pin);if(next!=raw){raw=next;changed=millis();}
   if((uint32_t)(millis()-changed)>=35 && next!=stable){stable=next;return !stable;}return false;}
};
Key selectKey,backKey,confirmKey;

void asset(int x,int y,const uint16_t* image,int w,int h){display->drawRGBBitmap(x,y,image,w,h);}
void textAt(int x,int y,const char* label,uint16_t color,uint8_t size=1){
 display->setTextWrap(false);display->setTextSize(size);display->setTextColor(color);display->setCursor(x,y);display->print(label);
}
void bottom(const char* hint){
 display->fillRect(0,114,128,14,BG);display->drawFastHLine(5,113,118,LINE);
 textAt(7,118,hint,MUTED);
}
void header(const char* label){
 display->fillRect(0,0,128,24,BG);
 display->fillRoundRect(5,4,20,20,3,WHITE);
 asset(5,4,SMALL_ICON,20,20);
 textAt(31,8,label,WHITE);
 display->drawFastHLine(5,26,118,LINE);
}
void splash(){
  display->fillScreen(BG);
  // Layout is fixed to the visible 128 x 128 ST7735 viewport.
  // Leave distinct vertical bands for the logo, name and subtitle.
  display->fillRoundRect(27,3,74,74,7,WHITE);
  asset(29,5,BRAND_ICON,70,70);

  const char* first="Maker";
  const char* second="Vault";
  display->setFont(&FreeSansBold9pt7b);
  display->setTextSize(1);
  int16_t ax,ay,bx,by;
  uint16_t aw,ah,bw,bh;
  display->getTextBounds(first,0,0,&ax,&ay,&aw,&ah);
  display->getTextBounds(second,0,0,&bx,&by,&bw,&bh);
  const int16_t total=(int16_t)aw+(int16_t)bw;
  const int16_t x=(128-total)/2;
  const int16_t baseline=96;
  if(total<=122){
    display->setTextColor(WHITE);
    display->setCursor(x-ax,baseline);
    display->print(first);
    display->setTextColor(SKY);
    display->setCursor(x+(int16_t)aw-bx,baseline);
    display->print(second);
  }else{
    // Unexpected font metrics: use the native 6x8 font rather than crop.
    display->setFont(NULL);
    const int16_t nativeX=(128-10*6)/2;
    textAt(nativeX,85,first,WHITE);
    textAt(nativeX+5*6,85,second,SKY);
  }
  display->setFont(NULL);
  // 19 characters in Adafruit's default 6px font = 114px.
  // Derive the origin from measured bounds rather than a fixed X.
  const char* subtitle="RFID / NFC TERMINAL";
  int16_t sx,sy;uint16_t sw,sh;
  display->setTextSize(1);
  display->getTextBounds(subtitle,0,0,&sx,&sy,&sw,&sh);
  textAt((128-(int16_t)sw)/2-sx,107,subtitle,MUTED);
  display->drawRoundRect(18,120,92,4,2,LINE);
  display->fillRoundRect(19,121,90,2,1,BLUE);
  delay(380);
}

// Material-inspired raster assets are the sole icon source. Tint the 15x15
// status icons without restoring the old pixel-art line drawings.
// The source artwork uses a dark background and light foreground. Preserve
// antialiased edge coverage by scaling the requested RGB565 status colour.
void tintedIcon(int x,int y,const uint16_t* pixels,int width,int height,uint16_t tint){
 const uint8_t tr=((tint>>11)&31),tg=((tint>>5)&63),tb=(tint&31);
 for(int j=0;j<height;j++)for(int i=0;i<width;i++){
  uint16_t px=pgm_read_word(pixels+j*width+i);
  // Icon pixel brightness: near-background becomes transparent.
  uint8_t r=(px>>11)&31,g=(px>>5)&63,b=px&31;
  int coverage=max((int)r*2,max((int)g,(int)b*2))-6;
  if(coverage<0)coverage=0;
  if(coverage>58)coverage=58;
  if(coverage>2){
   const int br=(BG>>11)&31,bg=(BG>>5)&63,bb=BG&31;
   uint16_t out=(((br*(58-coverage)+tr*coverage)/58)<<11)|
                (((bg*(58-coverage)+tg*coverage)/58)<<5)|
                 ((bb*(58-coverage)+tb*coverage)/58);
   display->drawPixel(x+i,y+j,out);
  }
 }
}
void wifiStatusIcon(int x,int y,uint16_t c){tintedIcon(x,y,MV_WIFI_FOOTER,15,15,c);}
void mvStatusIcon(int x,int y,uint16_t c){tintedIcon(x,y,MV_SERVER_FOOTER,15,15,c);}
void centeredAt(int y,const char *label,uint16_t color){
 int16_t x0,y0;uint16_t w,h;
 display->getTextBounds(label,0,y,&x0,&y0,&w,&h);
 textAt((128-(int)w)/2-x0,y,label,color);
}
// Shared text layout: word wrap, character fallback for URLs, and scrollable details.
// Content occupies y=31..109; top header and bottom navigation remain fixed.
const int CONTENT_TOP=32, CONTENT_BOTTOM=109, ROW_H=12, MAX_LINES=64;
struct DetailLine {String content;uint16_t ink;};
DetailLine detailLines[MAX_LINES];
void detailClear(){detailCount=0;}
void detailLine(const String &line,uint16_t color){
 if(detailCount<MAX_LINES)detailLines[detailCount++]={line,color};
}
void detailWrapped(const String &raw,uint16_t color,int width=114){
 String word="",line="";
 auto flushWord=[&](){
  if(!word.length())return;
  while(word.length()){
   String candidate=line+word;int16_t xx,yy;uint16_t ww,hh;
   display->setTextSize(1);display->getTextBounds(candidate.c_str(),0,0,&xx,&yy,&ww,&hh);
   if(ww<=width){line=candidate;word="";break;}
   if(line.length()){detailLine(line,color);line="";continue;}
   String chunk="";
   while(word.length()){
    String test=chunk+word[0];display->getTextBounds(test.c_str(),0,0,&xx,&yy,&ww,&hh);
    if(ww>width && chunk.length())break;
    chunk+=word[0];word.remove(0,1);
   }
   detailLine(chunk,color);
  }
 };
 for(size_t i=0;i<raw.length();i++){
  char ch=raw[i];
  if(ch=='\n'){flushWord();detailLine(line,color);line="";continue;}
  if(ch==' '){flushWord();if(line.length())line+=' ';}else word+=ch;
 }
 flushWord();if(line.length())detailLine(line,color);
}
void detailPaint(const char* title,bool full=true){
 if(full){display->fillScreen(BG);header(title);}
 else display->fillRect(0,CONTENT_TOP,128,CONTENT_BOTTOM-CONTENT_TOP+1,BG);
 const int visible=(CONTENT_BOTTOM-CONTENT_TOP)/ROW_H;
 int maxScroll=max(0,detailCount-visible);
 detailScroll=constrain(detailScroll,0,maxScroll);
 for(int i=0;i<visible && i+detailScroll<detailCount;i++){
  int y=CONTENT_TOP+i*ROW_H;
  textAt(6,y,detailLines[i+detailScroll].content.c_str(),detailLines[i+detailScroll].ink);
 }
 if(maxScroll){
  display->drawFastVLine(124,CONTENT_TOP,CONTENT_BOTTOM-CONTENT_TOP,LINE);
  int thumb=max(8,(CONTENT_BOTTOM-CONTENT_TOP)*visible/detailCount);
  int yy=CONTENT_TOP+(CONTENT_BOTTOM-CONTENT_TOP-thumb)*detailScroll/maxScroll;
  display->fillRoundRect(123,yy,4,thumb,2,SKY);
 }
 bottom(maxScroll?"Turn: scroll  Back":"Back: menu");
}
void detailMove(int direction,const char* title){
 int visible=(CONTENT_BOTTOM-CONTENT_TOP)/ROW_H;
 int newScroll=constrain(detailScroll+direction,0,max(0,detailCount-visible));
 if(newScroll==detailScroll)return;
 detailScroll=newScroll;detailPaint(title,false);
}
void categoryRow(int i,const char* label){
 int y=42+i*29;
 display->fillRect(4,y,120,26,BG);
 if(!label[0])return;
 display->fillRoundRect(5,y,118,25,5,i==subSelected?NAVY:CARD);
 if(i==subSelected)display->fillRoundRect(5,y+4,3,17,1,SKY);
 textAt(13,y+9,label,i==subSelected?WHITE:SILVER);
}
void categoryPage(const char* title,const char* first,const char* second){
 display->fillScreen(BG);header(title);
 const char* labels[]={first,second};
 for(int i=0;i<2;i++){
  if(!labels[i][0])continue; // hide scoped actions until supported and granted
  categoryRow(i,labels[i]);
 }
 bottom("OK: open Back: home");
}
void menuGlyph(int idx,int x,int y,bool active){
 const uint16_t* iconsA[]={MV_SCAN_ACTIVE,MV_RECENT_ACTIVE,MV_CONNECTIONS_ACTIVE,MV_INFO_ACTIVE};
 const uint16_t* iconsB[]={MV_SCAN_INACTIVE,MV_RECENT_INACTIVE,MV_CONNECTIONS_INACTIVE,MV_INFO_INACTIVE};
 asset(x,y,active?iconsA[idx]:iconsB[idx],16,16);
}
void menuItem(int index){
 int y=32+index*20;bool active=index==selected;
 display->fillRect(3,y,122,19,BG);
 display->fillRoundRect(4,y,120,18,4,active?NAVY:CARD);
 if(active)display->fillRoundRect(4,y+3,3,12,1,SKY);
 menuGlyph(index,12,y+1,active);
 textAt(35,y+6,choices[index],active?WHITE:SILVER);
 if(active){display->drawLine(113,y+6,117,y+9,SKY);display->drawLine(117,y+9,113,y+12,SKY);}
}
bool readerReady(){return readerBusVerified || hasScanned;}
const char* readerLabel(){
 if(readerReady())return "Reader ready";
 if(readerInitDone)return "Reader check";
 return "Starting reader";
}
void home(){
 display->fillScreen(BG);header("MakerVault");
 for(int i=0;i<4;i++)menuItem(i);
 // Bottom bar: only the three status icons and short labels; no clipping.
 display->fillRect(0,113,128,15,BG);display->drawFastHLine(5,112,118,LINE);
 display->fillCircle(8,121,3,readerReady()?GREEN:(readerInitDone?AMBER:MUTED));
 textAt(14,118,"RFID",SILVER);
 // Live, colour-coded icons. The older bitmap artwork was permanently grey.
 wifiStatusIcon(51,113,WiFi.status()==WL_CONNECTED?GREEN:(knownCount?AMBER:MUTED));
 mvStatusIcon(86,113,mvCheckState==3?GREEN:(mvCheckState==2?AMBER:(mvCheckState==4?0xF800:MUTED)));
 // Connection details live in the Connections screen.
 shownWifiState=255;shownMvState=255;
}
void refreshFooter(){
 if(page!=HOME)return;
 uint8_t wifiState=WiFi.status()==WL_CONNECTED?2:(knownCount?1:0);
 uint8_t mvState=mvCheckState;
 if(wifiState==shownWifiState && mvState==shownMvState)return;
 shownWifiState=wifiState;shownMvState=mvState;
 display->fillRect(49,113,17,15,BG);display->fillRect(84,113,17,15,BG);
 wifiStatusIcon(51,113,wifiState==2?GREEN:(wifiState==1?AMBER:MUTED));
 mvStatusIcon(86,113,mvState==3?GREEN:(mvState==2?AMBER:(mvState==4?0xF800:MUTED)));
}
void simple(const char* title,const char* line1,const char* line2=""){
 display->fillScreen(BG);header(title);
 display->fillRoundRect(7,36,114,62,7,CARD);
 textAt(13,54,line1,WHITE);
 if(line2[0])textAt(13,72,line2,MUTED);
 bottom("Back: menu");
}
// Draw a prominent record identity, then the MakerVault ID; UID is secondary.
// Declared here because the scan screen reuses the record name wrapping helper.
int regWrapLines(const String &source, String *lines);
void scanPage(){
 display->fillScreen(BG);
 if(!scanResultVisible){
  header("Read tag");
  display->fillRoundRect(9,32,110,76,9,CARD);
  display->drawRoundRect(9,32,110,76,9,BLUE);
  asset(41,37,MV_SCAN_LARGE,46,46);
  centeredAt(78,"READY TO SCAN",WHITE);
  centeredAt(92,"Present a tag",SKY);
  bottom("Back: menu");return;
 }
 header("Tag detected");
 if(scanLookupState==2){
  String nameLines[20];
  int count=regWrapLines(scanLookupName,nameLines);
  int maxLines=min(3,count);
  for(int i=0;i<maxLines;i++)textAt(7,36+i*13,nameLines[i].c_str(),WHITE);
  // The full identifier must fit: MV- plus eight hex chars.
  textAt(7,79,scanLookupCode.c_str(),SKY);
 }else if(scanLookupState==3){
  // Material-style unknown identity: filled amber status disc with a
  // high-contrast white help glyph, rather than an outlined circle.
  display->fillCircle(64,54,20,AMBER);
  textAt(58,46,"?",WHITE,2);
  // Restore normal text size before centeredAt measures the label bounds.
  display->setTextSize(1);
  centeredAt(78,"Not",AMBER);
  centeredAt(90,"registered",AMBER);
 }else if(scanLookupState==1){
  centeredAt(62,"Checking tag...",SKY);
 }else if(scanLookupState==4){
  centeredAt(57,"Lookup unavailable",AMBER);
 }else{
  centeredAt(52,"TAG READ",GREEN);
  textAt(7,74,recentType.substring(0,19).c_str(),SILVER);
 }
 // UID remains visible, small and lower priority.
 textAt(7,100,recentUid.substring(0,19).c_str(),SILVER);
 bottom("Confirm: rescan");
}
void remoteLookupScreen(){
 display->fillScreen(BG);header("Find MakerTag");
 if(remoteLookupState==1){
  centeredAt(48,"Request received",GREEN);
  centeredAt(68,"Present a tag",SKY);
  bottom("Back: dismiss");
 }else if(remoteLookupState==2){
  centeredAt(60,"Looking up...",SKY);
  bottom("Please wait");
 }else if(remoteLookupState==3){
  centeredAt(48,"Lookup complete",GREEN);
  centeredAt(68,"Check MakerVault",SKY);
  bottom("Back: menu");
 }else{
  centeredAt(48,"Lookup failed",AMBER);
  textAt(7,71,remoteLookupError.substring(0,19).c_str(),SILVER);
  bottom("Back: menu");
 }
}
void connectionRow(int i){
 const char* menu[]={"Wi-Fi","MakerVault"};
 {
  int y=43+i*31;
  display->fillRect(5,y,118,27,BG);
  display->fillRoundRect(6,y,116,26,4,i==connSelected?NAVY:CARD);
  if(i==connSelected)display->fillRoundRect(6,y+3,3,20,1,SKY);
  if(i==0)wifiStatusIcon(11,y+4,WiFi.status()==WL_CONNECTED?GREEN:AMBER);
  else mvStatusIcon(11,y+4,mvPairVerified&&mvCheckState==3?GREEN:(mvCheckState==4?0xF800:AMBER));
  textAt(38,y+9,menu[i],i==connSelected?WHITE:SILVER);
 }
}
void connections(){
 display->fillScreen(BG);header("Connections");
 for(int i=0;i<2;i++)connectionRow(i);
 bottom("OK: open Back: home");
}
const int WIFI_MENU_COUNT=5;
const char* wifiOption(int i){
 if(i==0)return wifiManualOffline?"Connect":(WiFi.status()==WL_CONNECTED?"Disconnect":"Connect");
 if(i==1)return "Connection status";
 if(i==2)return "Known networks";
 if(i==3)return "Add network";
 return "Reconnect";
}
void wifiRow(int i){
 int y=51+i*12;
 display->fillRect(5,y,118,12,BG);
 if(i==wifiSelected)display->fillRoundRect(5,y,118,12,3,NAVY);
 textAt(10,y+2,wifiOption(i),i==wifiSelected?WHITE:SILVER);
}
void wifiMenu(){
 display->fillScreen(BG);header("Wi-Fi");
 textAt(7,31,wifiManualOffline?"Manually offline":(WiFi.status()==WL_CONNECTED?"Connected":"Offline"),WiFi.status()==WL_CONNECTED?GREEN:AMBER);
 String network=WiFi.status()==WL_CONNECTED?WiFi.SSID():"No active network";
 if(network.length()>19)network=network.substring(0,18)+"~";
 textAt(7,43,network.c_str(),SILVER);
 for(int i=0;i<WIFI_MENU_COUNT;i++)wifiRow(i);
 bottom("OK: open Back: menu");
}
// Values are read from the active Wi-Fi interface, never cached as a past lease.
String currentIpAddress(){
 return WiFi.status()==WL_CONNECTED?WiFi.localIP().toString():String("Not connected");
}
void wifiStatusPage(){
 const bool connected=WiFi.status()==WL_CONNECTED;
 detailClear();detailScroll=0;
 detailWrapped(String("Connection: ")+(connected?"Connected":(wifiManualOffline?"Manually offline":"Disconnected")),connected?GREEN:AMBER);
 detailWrapped("SSID: "+(connected?WiFi.SSID():String("Not connected")),SILVER);
 detailWrapped("IP address: "+currentIpAddress(),connected?WHITE:SILVER);
 if(connected)detailWrapped("Signal: "+String(WiFi.RSSI())+" dBm",SILVER);
 else detailWrapped("Signal: unavailable",MUTED);
 detailPaint("Wi-Fi status");
}
void makerRow(int i){
 const char* entries[]={"Connection status","Test HTTPS","Pair device","Server setup","Remove CA","Forget pairing"};
 int y=30+i*14;
 display->fillRect(5,y,118,13,BG);
 if(i==mvSelected)display->fillRoundRect(5,y,118,13,3,NAVY);
 textAt(10,y+3,entries[i],i==mvSelected?WHITE:SILVER);
}
void makerMenu(){
 display->fillScreen(BG);header("MakerVault");
 for(int i=0;i<6;i++)makerRow(i);
 bottom("OK: open Back: menu");
}
void removeCaScreen(){
 display->fillScreen(BG);header("Remove custom CA?");
 centeredAt(39,mvCaPem.isEmpty()?"No custom CA":"Switch to public CA",WHITE);
 centeredAt(56,"Key is retained",SILVER);
 centeredAt(73,"Private CA won't work",AMBER);
 bottom("OK: remove Back: no");
}
void forgetPairScreen(){
 display->fillScreen(BG);header("Forget pairing?");
 centeredAt(40,"Remove local key?",WHITE);
 centeredAt(57,"Wi-Fi is retained",SILVER);
 centeredAt(76,"Use MV to revoke",AMBER);
 bottom("OK: forget Back: no");
}
void mvSettings(){
 if(lanPortalActive && WiFi.status()==WL_CONNECTED){
  String link="http://"+WiFi.localIP().toString()+"/makervault";
  QRCode code;uint8_t data[qrcode_getBufferSize(5)];
  qrcode_initText(&code,data,5,ECC_LOW,link.c_str());
  display->fillScreen(BG);
  centeredAt(3,"MAKERVAULT SETUP",WHITE);
  int scale=2;int x=(128-(code.size+8)*scale)/2,y=17;
  display->fillRect(x,y,(code.size+8)*scale,(code.size+8)*scale,WHITE);
  for(int r=0;r<code.size;r++)for(int c=0;c<code.size;c++)
   if(qrcode_getModule(&code,c,r))display->fillRect(x+(c+4)*scale,y+(r+4)*scale,scale,scale,0);
  centeredAt(105,"SCAN LAN QR",SKY);
  String addr=WiFi.localIP().toString();
  centeredAt(117,addr.c_str(),WHITE);
 }else{
  // Unconnected terminal: join its temporary AP, then open the local portal.
  drawSetupQR(*display,BG,WHITE);
  display->fillRect(0,0,128,14,BG);
  centeredAt(3,"MAKERVAULT SETUP",WHITE);
  display->fillRect(0,105,128,23,BG);
  centeredAt(106,"192.168.4.1",SKY);
  centeredAt(118,"/makervault",WHITE);
 }
}
void mvStatusPage(){
 detailClear();
 detailWrapped(WiFi.status()==WL_CONNECTED?"Wi-Fi: connected":"Wi-Fi: offline",WiFi.status()==WL_CONNECTED?GREEN:AMBER);
 detailWrapped(String("Pairing: ")+mvPairingLabel(),mvPairVerified?GREEN:AMBER);
 if(mvDeviceName.length())detailWrapped("Device: "+mvDeviceName,WHITE);
 detailWrapped(String("API: ")+mvStatusName(),mvCheckState==3?GREEN:(mvCheckState==4?0xF800:AMBER));
 detailWrapped("HTTPS: "+String(mvTlsStatus==1?"Verified":(mvTlsStatus==2?"Certificate rejected":(mvTlsStatus==3?"Failed / unknown":"Not tested"))),mvTlsStatus==1?GREEN:AMBER);
 detailWrapped("Trust: "+mvTrustMode(),SILVER);
 detailWrapped("Last OK: "+mvLastGoodConnection,SILVER);
 detailWrapped("Last error: "+mvLastConnectionError,mvTlsStatus==1?SILVER:AMBER);
 detailWrapped("API permissions:",WHITE);
 detailWrapped(String("MakerTag read: ")+(mvCanReadTags?"Allowed":"Not granted"),mvCanReadTags?GREEN:SILVER);
 detailWrapped(String("MakerTag write: ")+(mvCanWriteTags?"Allowed":"Not granted"),mvCanWriteTags?GREEN:SILVER);
 detailWrapped(String("Inventory search: ")+(mvCanSearchInventory?"Allowed":"Not granted"),mvCanSearchInventory?GREEN:SILVER);
 detailWrapped(String("Heartbeat: ")+(mvCanHeartbeat?"Allowed":"Not granted"),mvCanHeartbeat?GREEN:SILVER);
 detailWrapped("Tag writing: not installed",MUTED);
 detailWrapped(String("Server: ")+mvBaseUrl,SILVER);
 if(mvLastHttpCode)detailWrapped("HTTP: "+String(mvLastHttpCode),mvLastHttpCode==200?GREEN:AMBER);
 detailPaint("MakerVault link");
}
void mvPairPage(){
 display->fillScreen(BG);header("Pair device");
 if(WiFi.status()!=WL_CONNECTED)textAt(8,37,"Connect Wi-Fi first",AMBER);
 else if(!mvValidUrl(mvBaseUrl)){
  textAt(8,37,"Set HTTPS server",AMBER);textAt(8,52,"in MakerVault setup",SILVER);
 }else if(mvPairState==1){centeredAt(43,"Connecting securely",SKY);centeredAt(60,"Requesting code...",SILVER);}
 else if(mvPairState==2){
  centeredAt(38,"Enter in MakerVault",SILVER);
  display->setTextSize(2);centeredAt(59,mvPairCode.c_str(),WHITE);display->setTextSize(1);
  uint32_t elapsed=(uint32_t)(millis()-mvPairStarted);uint32_t left=elapsed>=300000?0:(300000-elapsed)/1000;
  centeredAt(88,("Expires in "+String(left)+"s").c_str(),AMBER);
 }else if(mvPairState==3){centeredAt(43, mvCheckState==3?"Connected!":"Paired",GREEN);centeredAt(64,mvCheckState==3?"MakerVault ready":"Verifying API",SILVER);}
 else if(mvPairState==4||mvPairState==5){
  centeredAt(34,"Pairing failed",AMBER);
  // Negative HTTPClient codes are not HTTP response codes. Show the
  // verified TLS failure reason rather than a misleading 'HTTP -1'.
  String error=mvPairError;
  if(error.startsWith("Start HTTP -")){
    if(mvTlsStatus==2)error="Certificate rejected";
    else if(mvLastConnectionError=="Cannot resolve server")error="DNS lookup failed";
    else if(mvLastConnectionError=="Wi-Fi offline")error="Wi-Fi disconnected";
    else if(mvLastConnectionError=="Server timeout")error="Server timed out";
    else if(mvLastConnectionError=="Clock unavailable")error="Clock unavailable";
    else error="HTTPS connection failed";
  }
  // Basic 128px display: two readable lines, with full error in Serial.
  if(error.length()>23){
    int split=error.lastIndexOf(' ',23);
    if(split<7)split=22;
    String top=error.substring(0,split);
    String rest=error.substring(split);rest.trim();
    centeredAt(57,top.c_str(),SILVER);
    if(rest.length()>23)rest=rest.substring(0,22)+"~";
    centeredAt(70,rest.c_str(),SILVER);
  }else centeredAt(61,error.c_str(),SILVER);
  centeredAt(92,"Check CA / server",MUTED);
 }
 else textAt(8,43,"Press OK to start",SILVER);
 bottom(mvPairState==3?"Back: menu":"OK: retry Back: stop");
}
void knownNetworkRow(uint8_t i){
 {
  uint8_t y=32+i*15;
  display->fillRect(6,y,116,14,BG);
  if(i==networkSelected)display->fillRoundRect(6,y,116,14,3,NAVY);
  String label=knownSSID[i];if(label.length()>17)label=label.substring(0,16)+"~";
  textAt(10,y+3,label.c_str(),WHITE);
  if(WiFi.status()==WL_CONNECTED && WiFi.SSID()==knownSSID[i])display->fillCircle(116,y+7,3,GREEN);
 }
}
void knownNetworks(){
 display->fillScreen(BG);header("Known Wi-Fi");
 if(knownCount==0)textAt(10,44,"No saved networks",MUTED);
 for(uint8_t i=0;i<knownCount;i++)knownNetworkRow(i);
 bottom("OK: join  Knob: del");
}


void deleteNetworkScreen(){
 display->fillScreen(BG);header("Forget Wi-Fi?");
 display->fillRoundRect(7,37,114,62,7,CARD);
 textAt(13,45,"Remove network?",WHITE);
 String label=pendingDeleteSSID;
 if(label.length()>18)label=label.substring(0,17)+"~";
 textAt(13,63,label.c_str(),SKY);
 textAt(13,83,"Cannot undo",AMBER);
 bottom("OK: delete  Back: no");
}
void connectKnownNetwork(){
 wifiSetManualOffline(false);
 if(!knownCount || networkSelected>=knownCount)return;
 const String ssid=knownSSID[networkSelected];
 if(WiFi.status()==WL_CONNECTED && WiFi.SSID()==ssid){
  Serial.println("[WIFI] Already connected to selected network");
  return;
 }
 Serial.print("[WIFI] Connecting to saved network: ");Serial.println(ssid);
 // Explicit connection first, before allowing the periodic fallback scanner.
 WiFi.mode(WIFI_STA);
 WiFi.disconnect();
 lastReconnectTry=millis();
 WiFi.begin(knownSSID[networkSelected].c_str(),knownPass[networkSelected].c_str());
 render();
}

void info(){
 detailClear();detailWrapped(String("Firmware v")+MAKERVAULT_FIRMWARE_VERSION,WHITE);
 detailWrapped("Device: "+(mvDeviceName.length()?mvDeviceName:String("Not verified")),SILVER);
 detailWrapped("Wi-Fi IP: "+currentIpAddress(),WiFi.status()==WL_CONNECTED?WHITE:SILVER);
 detailWrapped("Pairing: "+String(mvPairingLabel()),mvPairVerified?GREEN:AMBER);
 detailWrapped(String("API scopes: ")+(mvCheckState==3?"verified":"unknown"),mvCheckState==3?GREEN:AMBER);
 detailWrapped(String("Tag read: ")+(mvCanReadTags?"allowed":"unavailable"),mvCanReadTags?GREEN:SILVER);
 detailWrapped(String("Tag write: ")+(mvCanWriteTags?"allowed":"unavailable"),mvCanWriteTags?GREEN:SILVER);
 detailWrapped("SPI: shared hardware",SILVER);
 detailWrapped("RC522: 0x"+String(versionReg,HEX),SILVER);
 detailWrapped(readerLabel(),readerReady()?GREEN:AMBER);
 detailPaint("System info");
}
void mvTlsTestPage(){
 detailClear();
 if(mvTestRunning){
  detailWrapped("Running HTTPS test...",SKY);
  detailWrapped("No pairing changes",SILVER);
 }else{
  const int code=mvTestHttpCode;
  detailWrapped(String("Trust: ")+mvTestTrust,SILVER);
  detailWrapped(String("TCP/TLS: ")+(mvTestTlsStatus==1?"Verified":(mvTestTlsStatus==2?"Certificate rejected":"Failed / not confirmed")),mvTestTlsStatus==1?GREEN:AMBER);
  detailWrapped(String("API: ")+(code>0?mvHttpLabel(code):String("No HTTP response")),code==200?GREEN:AMBER);
  detailWrapped("Result: "+mvTestError,code==200?GREEN:AMBER);
  if(mvTestDetail.length())detailWrapped("Diagnostic: "+mvTestDetail,SILVER);
 }
 detailPaint("Test HTTPS");
}
// The ST7735S has 128x128 pixels. Default GFX text is 6px per character;
// x=7 gives room for 19 characters without touching the right edge.
constexpr int REG_TEXT_CHARS=19;
constexpr int REG_MAX_LINES=20;

// Split at word boundaries; unusually long words are broken at the margin.
// A label may exceed one screen, so page through all its lines every 4s.
int regWrapLines(const String &source, String *lines) {
 String pending=source;
 pending.trim();
 int count=0;
 while(pending.length() && count<REG_MAX_LINES) {
  int cut=min((int)pending.length(),REG_TEXT_CHARS);
  if(cut<(int)pending.length()) {
   int boundary=pending.lastIndexOf(' ',cut);
   if(boundary>0)cut=boundary;
  }
  String line=pending.substring(0,cut);
  line.trim();
  if(line.length())lines[count++]=line;
  pending=pending.substring(cut);
  pending.trim();
 }
 return count;
}

void regDisplayRecord() {
 String lines[REG_MAX_LINES];
 const int count=regWrapLines(regTarget,lines);
 const int pages=max(1,(count+2)/3);
 const int page=(millis()/4000UL)%pages;
 for(int i=0;i<3;i++) {
  int index=page*3+i;
  if(index<count)textAt(7,39+i*13,lines[index].c_str(),WHITE);
 }
 if(pages>1) {
  String indicator=String(page+1)+"/"+String(pages);
  textAt(100,29,indicator.c_str(),MUTED);
 }
 // Two lines reserved for the record ID, separate from the record name.
 if(regRecordCode.length()) {
  String code="ID: "+regRecordCode;
  textAt(7,79,code.substring(0,REG_TEXT_CHARS).c_str(),SILVER);
  if(code.length()>REG_TEXT_CHARS)
   textAt(7,90,code.substring(REG_TEXT_CHARS,REG_TEXT_CHARS*2).c_str(),SILVER);
 }
 textAt(7,103,"Present a tag",SKY);
 bottom("Back: dismiss");
}

void registrationScreen(){
 display->fillScreen(BG);
 header("Assign MakerTag");
 if(regState==1) {
  regDisplayRecord();
 } else if(regState==2) {
  textAt(7,37,"Scanned UID:",SILVER);
  // 10-byte UIDs are 20 hexadecimal characters; show ALL bytes.
  textAt(7,52,regUid.substring(0,REG_TEXT_CHARS).c_str(),WHITE);
  if(regUid.length()>REG_TEXT_CHARS)
   textAt(7,65,regUid.substring(REG_TEXT_CHARS,REG_TEXT_CHARS*2).c_str(),WHITE);
  textAt(7,87,"Assign this tag?",SKY);
  bottom("OK: save Back: no");
 } else if(regState==6) {
  // Both names wrap at word boundaries. Long labels cycle through their
  // remaining lines rather than silently clipping the destination.
  textAt(7,30,"Already registered",AMBER);
  String oldLines[REG_MAX_LINES],newLines[REG_MAX_LINES];
  int oldCount=regWrapLines(regPreviousLabel,oldLines);
  int newCount=regWrapLines(regTarget,newLines);
  int oldPages=max(1,(oldCount+1)/2),newPages=max(1,(newCount+1)/2);
  int pageNumber=(millis()/4000UL)%max(oldPages,newPages);
  for(int i=0;i<2;i++){
   int oldIndex=(pageNumber%oldPages)*2+i;
   if(oldIndex<oldCount)textAt(7,43+i*11,oldLines[oldIndex].c_str(),WHITE);
  }
  textAt(7,66,"Reassign to:",SILVER);
  for(int i=0;i<2;i++){
   int newIndex=(pageNumber%newPages)*2+i;
   if(newIndex<newCount)textAt(7,79+i*11,newLines[newIndex].c_str(),WHITE);
  }
  bottom("OK: move Back: no");
 } else if(regState==3) {
  textAt(7,54,"Registering...",SKY);
  bottom("Please wait");
 } else if(regState==4) {
  textAt(7,48,"Tag assigned",GREEN);
  bottom("Back: menu");
 } else {
  textAt(7,43,"Registration failed",AMBER);
  String errors[REG_MAX_LINES];
  int count=regWrapLines(regError,errors);
  for(int i=0;i<count && i<3;i++)
   textAt(7,58+i*13,errors[i].c_str(),WHITE);
  bottom("Back: menu");
 }
}
#if MAKERVAULT_NDEF_WRITER_EXPERIMENTAL
void ndefWriterScreen(){
 detailClear();detailWrapped("Tag: "+ndefTagId,WHITE);
 if(ndefState==1)detailWrapped("Present matching NTAG215",SILVER);
 else if(ndefState==2){detailWrapped(ndefExistingUrl.length()?"Overwrite existing URL?":"Write new NFC URL?",AMBER);if(ndefExistingUrl.length())detailWrapped("Current: "+ndefExistingUrl,SILVER);detailWrapped("New: "+ndefUrl,SILVER);}
 else if(ndefState==3)detailWrapped("Checking server authorization",SILVER);
 else if(ndefState==4)detailWrapped("Present tag again to write",AMBER);
 else if(ndefState==5)detailWrapped("Reporting read-back verification",SILVER);
 else if(ndefState==6)detailWrapped("Write verified by MakerVault",GREEN);
 else if(ndefState==7)detailWrapped(ndefError,AMBER);
 else if(ndefState==8)detailWrapped("Present tag to verify",SILVER);
 detailScroll=0;detailPaint("NTAG215 write");
}
#endif
void render(){
 switch(page){
  case HOME:home();break;
  case SCAN:scanPage();break;
  case REGISTRATION:registrationScreen();break;
  case LOOKUP_REQUEST:remoteLookupScreen();break;
#if MAKERVAULT_NDEF_WRITER_EXPERIMENTAL
  case NDEF_WRITE:ndefWriterScreen();break;
#endif
  case RECENT: simple("Recent tag","Not available");break;
  case CONTROLS:simple("Control test","Turn / press", "Back exits");break;
  case CONNECTIONS:connections();break;
  case WIFI_MENU:wifiMenu();break;
  case MV_MENU:makerMenu();break;
  case MV_FORGET:forgetPairScreen();break;
  case MV_CA_REMOVE:removeCaScreen();break;
  case KNOWN_WIFI:knownNetworks();break;
  case DELETE_WIFI:deleteNetworkScreen();break;
  case MAKERVAULT_SETUP:mvSettings();break;
  case MV_STATUS:mvStatusPage();break;
  case MV_TLS_TEST:mvTlsTestPage();break;
  case MV_PAIR:mvPairPage();break;
  case SETUP_WIFI:drawSetupQR(*display,BG,WHITE);break;
  case INFO:info();break;
  case WIFI_STATUS:wifiStatusPage();break;
  case MAKERTAGS_MENU:categoryPage("MakerTags","Scan MakerTag",categoryCount()>1?"Create MakerTag":"");break;
  case RFID_MENU:categoryPage("RFID Tags","Read RFID tag",categoryCount()>1?"Write RFID tag":"");break;
  case UNAVAILABLE:simple(unavailableTitle.c_str(),"Coming later","No tag written");break;
 }
}
void control(const char* event){
 Serial.print("[CONTROL] ");Serial.println(event);
 if(page==CONTROLS){display->fillRoundRect(10,102,108,10,2,NAVY);textAt(14,103,event,WHITE);feedbackUntil=millis()+850;}
}
int categoryCount(){
 if(page==MAKERTAGS_MENU)return 1+int(MAKERTAG_CREATE_IMPLEMENTED && mvCanWriteTags && mvPairVerified);
 if(page==RFID_MENU)return 1+int(RFID_WRITE_IMPLEMENTED && mvCanWriteTags && mvPairVerified);
 return 1;
}
void move(int dir){
 if(page==MV_STATUS){detailMove(dir,"MakerVault link");return;}
 if(page==MV_TLS_TEST){detailMove(dir,"Test HTTPS");return;}
 if(page==WIFI_STATUS){detailMove(dir,"Wi-Fi status");return;}
 if(page==INFO){detailMove(dir,"System info");return;}
 if(page==MAKERTAGS_MENU||page==RFID_MENU){int old=subSelected;subSelected=(subSelected+dir+categoryCount())%categoryCount();
  if(old!=subSelected){const char* first=page==MAKERTAGS_MENU?"Scan MakerTag":"Read RFID tag";
   const char* second=page==MAKERTAGS_MENU?"Create MakerTag":"Write RFID tag";
   categoryRow(old,old==0?first:second);categoryRow(subSelected,subSelected==0?first:second);}
  return;}
 if(page==CONTROLS){control(dir>0?"Clockwise":"Anticlockwise");return;}
 if(page==CONNECTIONS){int old=connSelected;connSelected=(connSelected+dir+2)%2;connectionRow(old);connectionRow(connSelected);return;}
 if(page==WIFI_MENU){int old=wifiSelected;wifiSelected=(wifiSelected+dir+WIFI_MENU_COUNT)%WIFI_MENU_COUNT;wifiRow(old);wifiRow(wifiSelected);return;}
 if(page==MV_MENU){int old=mvSelected;mvSelected=(mvSelected+dir+6)%6;makerRow(old);makerRow(mvSelected);return;}
 if(page==KNOWN_WIFI){if(knownCount){int old=networkSelected;networkSelected=(networkSelected+dir+knownCount)%knownCount;knownNetworkRow(old);knownNetworkRow(networkSelected);}return;}
 if(page!=HOME)return;
 int old=selected;selected=(selected+dir+4)%4;
 menuItem(old);menuItem(selected);
 Serial.print("[MENU] ");Serial.println(choices[selected]);
}
void choose(){
 if(page==MAKERTAGS_MENU||page==RFID_MENU){
  if(subSelected==0){page=SCAN;scanResultVisible=false;render();}
  else if(subSelected<categoryCount()){
   unavailableTitle="Operation unavailable";page=UNAVAILABLE;render();
  }
  return;
 }
 if(page==CONTROLS){control("Select");return;}
 if(page==CONNECTIONS){page=connSelected==0?WIFI_MENU:MV_MENU;render();return;}
 if(page==WIFI_MENU){
  if(wifiSelected==0){
   if(wifiManualOffline || WiFi.status()!=WL_CONNECTED)wifiSetManualOffline(false);
   else wifiSetManualOffline(true);
   render();
  }else if(wifiSelected==1){page=WIFI_STATUS;render();}
  else if(wifiSelected==2){networkSelected=0;page=KNOWN_WIFI;render();}
  else if(wifiSelected==3){wifiSetManualOffline(false);beginPortal();if(setupAPActive){page=SETUP_WIFI;render();}}
  else {wifiSetManualOffline(false);if(WiFi.status()!=WL_CONNECTED){lastReconnectTry=millis()-12001;}else mvRequestCheck();render();}
  return;
 }
 if(page==MV_MENU){
  if(mvSelected==0){detailScroll=0;page=MV_STATUS;render();}
  else if(mvSelected==1){detailScroll=0;mvStartHttpsTest();page=MV_TLS_TEST;render();}
  else if(mvSelected==2){page=MV_PAIR;if(mvPairState!=2)mvStartPairing();render();}
  else if(mvSelected==3){beginMakerVaultPortal();if(setupPortalRunning()){page=MAKERVAULT_SETUP;render();}}
  else if(mvSelected==4){page=MV_CA_REMOVE;render();}
  else {page=MV_FORGET;render();}
  return;
 }
 if(page==KNOWN_WIFI){connectKnownNetwork();return;}
 if(page==MV_CA_REMOVE){
  if(mvRemoveCustomCA()){page=MV_MENU;render();}
  else {simple("CA removal", "Busy or save failed", "Back to menu");page=MV_MENU;}
  return;
 }
 if(page==MV_FORGET){if(mvForgetLocalPairing()){page=MV_MENU;render();}else {page=MV_MENU;render();}return;}
 if(page==MV_PAIR){if(mvPairState!=2)mvStartPairing();render();return;}
 if(page==MV_STATUS){mvRequestCheck();render();return;}
 if(page!=HOME)return;
 page=(selected==0)?MAKERTAGS_MENU:(selected==1?RFID_MENU:(selected==2?CONNECTIONS:INFO));subSelected=0;
 Serial.print("[MENU] Selected: ");Serial.println(choices[selected]);render();
}
void back(){
#if MAKERVAULT_NDEF_WRITER_EXPERIMENTAL
 if(page==NDEF_WRITE){ndefReset(ndefState!=6);page=MAKERTAGS_MENU;render();return;}
#endif
 if(page==LOOKUP_REQUEST && remoteLookupState!=2){remoteLookupReset();page=MAKERTAGS_MENU;render();return;}
 if(page==REGISTRATION && regState==3){Serial.println("[REGISTRATION] Request still submitting; Back cannot interrupt network task");registrationScreen();return;}
 if(page==REGISTRATION && regState!=3){regCancel();page=MAKERTAGS_MENU;render();return;}
 Serial.println("[BUTTON] BACK");if(page==DELETE_WIFI){pendingDeleteSSID="";page=KNOWN_WIFI;render();return;}if(page==SETUP_WIFI || page==MAKERVAULT_SETUP){if(lanPortalActive)stopPortal();page=(page==SETUP_WIFI)?WIFI_MENU:MV_MENU;render();return;}if(page==KNOWN_WIFI||page==WIFI_STATUS){page=WIFI_MENU;render();return;}
 if(page==MV_PAIR){if(mvPairState==1||mvPairState==2)mvCancelPairing();page=MV_MENU;render();return;}
 if(page==SCAN){page=selected==0?MAKERTAGS_MENU:RFID_MENU;render();return;}
 if(page==UNAVAILABLE){page=selected==0?MAKERTAGS_MENU:RFID_MENU;render();return;}
 if(page==MV_STATUS||page==MV_TLS_TEST||page==MV_PAIR||page==MV_FORGET||page==MV_CA_REMOVE){page=MV_MENU;render();return;}
 if(page==WIFI_MENU||page==MV_MENU){page=CONNECTIONS;render();return;}if(page!=HOME){page=HOME;render();}
}
void processInputs(){
 // Only meaningful menu input defers reader setup.
 const int oldSelection=selected; const Page oldPage=page;
 uint8_t ab=(digitalRead(ENC_A)<<1)|digitalRead(ENC_B);
 if(ab!=previousAB){lastEncoderTurnAt=millis();stepCount+=decode[(previousAB<<2)|ab];previousAB=ab;
  if(stepCount>=2||stepCount<=-2){int dir=stepCount>0?1:-1;stepCount=0;move(dir);}}
 const bool selectPressed=selectKey.pressed();
 // KY-040 push-switch contact can briefly chatter while its shaft rotates.
 // Consume the debounced event, but never turn rotation into a select action.
 if(selectPressed && (uint32_t)(millis()-lastEncoderTurnAt)>=180){
  Serial.println("[BUTTON] SELECT");
  if(page==KNOWN_WIFI && knownCount && networkSelected<knownCount){
   pendingDeleteSSID=knownSSID[networkSelected];page=DELETE_WIFI;render();
  }else if(page!=DELETE_WIFI)choose();
 }
 if(backKey.pressed())back();
 if(confirmKey.pressed()){
  Serial.println("[BUTTON] CONFIRM");
#if MAKERVAULT_NDEF_WRITER_EXPERIMENTAL
  if(page==NDEF_WRITE){if(ndefState==2)ndefConfirm();else if(ndefState==6||ndefState==7){ndefReset(false);page=MAKERTAGS_MENU;}render();return;}
#endif
  if(page==REGISTRATION && (regState==2||regState==6)){regSubmit(regUid,regKind);registrationScreen();return;}
  if(page==HOME)choose();else if(page==CONTROLS)control("Confirm");
  else if(page==SCAN){scanResultVisible=false;scanLookupQueued=false;scanLookupState=0;scanPage();}
  else if(page==DELETE_WIFI){
   // Resolve by SSID, not list index, to avoid deleting the wrong network.
   for(uint8_t i=0;i<knownCount;i++){
    if(knownSSID[i]==pendingDeleteSSID){forgetNetwork(i);break;}
   }
   pendingDeleteSSID="";
   if(networkSelected>=knownCount && networkSelected>0)networkSelected--;
   page=KNOWN_WIFI;render();
  }
  else if(page==CONNECTIONS||page==WIFI_MENU||page==MV_MENU||page==MV_FORGET||page==MV_CA_REMOVE||page==KNOWN_WIFI||page==MV_PAIR||page==MV_STATUS||page==MV_TLS_TEST||page==MAKERTAGS_MENU||page==RFID_MENU){choose();}
 }
 if(feedbackUntil&&(int32_t)(millis()-feedbackUntil)>=0){feedbackUntil=0;if(page==CONTROLS)render();}
 if(oldSelection!=selected || oldPage!=page || !digitalRead(ENC_SW) || !digitalRead(BACK_PIN) || !digitalRead(CONFIRM_PIN))lastInputAt=millis();
}
void readerInit(){
 Serial.println("[RFID] Initialising reader");
 digitalWrite(TFT_CS,HIGH);digitalWrite(RFID_CS,HIGH);
 rfid.PCD_Init();
 versionReg=rfid.PCD_ReadRegister(MFRC522::VersionReg);
 Serial.printf("[RFID] Version = 0x%02X\n",versionReg);
 knownReader=(versionReg==0x88||versionReg==0x90||versionReg==0x91||versionReg==0x92);
 if(!knownReader){
   byte check=rfid.PCD_ReadRegister(MFRC522::VersionReg);
   Serial.printf("[RFID] Verification = 0x%02X\n",check);
   if(check==versionReg)Serial.println("[RFID] Unrecognised but stable version");
 }
 // Hardware communications check. Reversibly change a reader timer register,
 // read it back, then restore. This never writes to a card/tag.
 // Useful for compatible RC522 clones with nonstandard VersionReg values.
 const byte saved=rfid.PCD_ReadRegister(MFRC522::TReloadRegL);
 const byte probe=(saved==0x5A)?0xA5:0x5A;
 rfid.PCD_WriteRegister(MFRC522::TReloadRegL,probe);
 const byte seen=rfid.PCD_ReadRegister(MFRC522::TReloadRegL);
 rfid.PCD_WriteRegister(MFRC522::TReloadRegL,saved);
 const byte restored=rfid.PCD_ReadRegister(MFRC522::TReloadRegL);
 readerBusVerified=(seen==probe && restored==saved &&
                    versionReg!=0x00 && versionReg!=0xFF);
 Serial.printf("[RFID] SPI register probe: %s (0x%02X => 0x%02X => 0x%02X)\n",
               readerBusVerified?"PASS":"FAIL",saved,seen,restored);
 readerInitDone=true;
 Serial.printf("[SYSTEM] Reader check complete at %lu ms\n",millis()-bootAt);
 // Update just the RFID status dot, not the entire home screen.
 // A full redraw here previously made input seem unresponsive.
 if(page==HOME){
  display->fillRect(4,117,9,9,BG);
  display->fillCircle(8,121,3,readerReady()?GREEN:AMBER);
 }
}

void pollTag(){
#if MAKERVAULT_NDEF_WRITER_EXPERIMENTAL
 const bool ndefScanning=(page==NDEF_WRITE && (ndefState==1 || ndefState==4 || ndefState==8));
 if(!ndefScanning && ndefState!=0 && page==NDEF_WRITE)return;
#else
 const bool ndefScanning=false;
#endif
 if((!ndefScanning && page!=SCAN && !(page==REGISTRATION && regState==1) && !(page==LOOKUP_REQUEST && remoteLookupState==1))||(page==SCAN && scanResultVisible)||(uint32_t)(millis()-lastPoll)<100)return;
 lastPoll=millis();digitalWrite(TFT_CS,HIGH);
 if(!rfid.PICC_IsNewCardPresent()||!rfid.PICC_ReadCardSerial())return;
 String uid="";
 for(byte i=0;i<rfid.uid.size;i++){
  if(rfid.uid.uidByte[i]<16)uid+='0';uid+=String(rfid.uid.uidByte[i],HEX);
 }
 uid.toUpperCase();recentUid=uid;
 recentType=String(rfid.PICC_GetTypeName(rfid.PICC_GetType(rfid.uid.sak)));
#if MAKERVAULT_NDEF_WRITER_EXPERIMENTAL
 if(ndefScanning){
  if(ndefState==8){
   MakerVaultRc522PageIO io(rfid);
   uint8_t fresh[MakerVaultNtag215::kUserBytes]={};
   if(!MakerVaultNtag215::readNdefArea(io,fresh) ||
      !MakerVaultNtag215::verifyReadback(rfid.uid.uidByte,rfid.uid.size,ndefUid.c_str(),ndefUrl.c_str(),fresh,sizeof(fresh))){
     ndefError="Fresh read-back failed";ndefState=7;
   }else ndefState=5;
  }else if(ndefState==1){
   MakerVaultRc522PageIO io(rfid);
   MakerVaultNtag215::MemoryInfo info={false,false,0};
   uint8_t area[MakerVaultNtag215::kUserBytes]={};
   if(rfid.uid.size!=7 || !MakerVaultNtag215::matchesUid(
        rfid.uid.uidByte,rfid.uid.size,ndefUid.c_str())){
    ndefError="Wrong tag UID";ndefState=7;
   }else if(!MakerVaultNtag215::inspectMemory(io,rfid.uid.uidByte,info)){
    ndefError="Cannot read tag config";ndefState=7;
   }else if(!info.supported){
    ndefError="Not NTAG215 (CC)";ndefState=7;
   }else if(!info.writable){
    ndefError="Locked / protected tag";ndefState=7;
   }else if(!MakerVaultNtag215::readNdefArea(io,area)){
    ndefError="Cannot read tag memory";ndefState=7;
   }else{
    const auto decision=MakerVaultNtag215::assess(
      info,rfid.uid.uidByte,rfid.uid.size,ndefUid.c_str(),
      ndefTagId.c_str(),ndefUrl.c_str(),area,sizeof(area),true,true);
    if(decision==MakerVaultNtag215::WRITE_READY){
      char existingUrl[MakerVaultNdef::kMaxUriLength+1]={};
      ndefExistingUrl=MakerVaultNdef::decodeUri(area,sizeof(area),existingUrl,sizeof(existingUrl))
        ? String(existingUrl) : String("");
      ndefState=2;
    }
    else{
      ndefError=decision==MakerVaultNtag215::WRITE_REJECT_FOREIGN_CONTENT
        ?"Existing NDEF needs override"
        :decision==MakerVaultNtag215::WRITE_REJECT_URL
        ?"Invalid MakerVault URL"
        :decision==MakerVaultNtag215::WRITE_REJECT_UID
        ?"Wrong tag UID":"Tag safety check failed";
      ndefState=7;
    }
   }
   if(ndefState==7)Serial.printf("[NDEF] Pre-write inspection: %s\\n",ndefError.c_str());
  }else if(ndefState==4){
   MakerVaultRc522PageIO io(rfid);
   if(rfid.uid.size!=7 || !mvPairVerified || !mvCanWriteTags ||
      (uint32_t)(millis()-ndefAuthorizedAt)>15000UL){ndefError="UID or authorization expired";ndefState=7;}
   else {
    const auto result=MakerVaultNtag215::writeNdef(io,rfid.uid.uidByte,ndefUid.c_str(),ndefTagId.c_str(),ndefUrl.c_str(),true,true);
    if(result!=MakerVaultNtag215::RESULT_VERIFIED){ndefError="Write/read-back failed";ndefState=7;}
    else ndefState=8;
   }
  }
  rfid.PICC_HaltA();rfid.PCD_StopCrypto1();render();
  if(ndefState==5)ndefSubmitVerified();
  return;
 }
#endif
 rfid.PICC_HaltA();rfid.PCD_StopCrypto1();
 if(page==LOOKUP_REQUEST){
  auto family=rfid.PICC_GetType(rfid.uid.sak);
  remoteLookupSubmit(uid,family==MFRC522::PICC_TYPE_MIFARE_UL?"nfc":"rfid");
  remoteLookupScreen();return;
 }
 if(page==REGISTRATION){
  // RC522 card-family detection cannot distinguish NTAG215 from all
  // other Type 2 / Ultralight-compatible tags. Validate model separately
  // before enabling any tag-memory write capability.
  MFRC522::PICC_Type family=rfid.PICC_GetType(rfid.uid.sak);
  if(family!=MFRC522::PICC_TYPE_MIFARE_UL && family!=MFRC522::PICC_TYPE_MIFARE_1K){
   regError="Unsupported tag";regState=5;registrationScreen();return;
  }
  regUid=uid;
  regKind=(family==MFRC522::PICC_TYPE_MIFARE_UL?"nfc":"rfid");
  regState=2;registrationScreen();return;
 }
 hasScanned=true;scanResultVisible=true;
 scanLookupBegin(recentUid,rfid.PICC_GetType(rfid.uid.sak)==MFRC522::PICC_TYPE_MIFARE_UL?"nfc":"rfid");
 Serial.printf("[RFID] UID=%s, Type=%s\n",recentUid.c_str(),recentType.c_str());
 scanPage();
}
void setup(){
 bootAt=millis();Serial.begin(115200);
 led.begin();led.setBrightness(0);led.clear();led.show();
 pinMode(TFT_CS,OUTPUT);digitalWrite(TFT_CS,HIGH);
 pinMode(RFID_CS,OUTPUT);digitalWrite(RFID_CS,HIGH);
 pinMode(ENC_A,INPUT_PULLUP);pinMode(ENC_B,INPUT_PULLUP);
 selectKey.begin(ENC_SW);backKey.begin(BACK_PIN);confirmKey.begin(CONFIRM_PIN);
 previousAB=(digitalRead(ENC_A)<<1)|digitalRead(ENC_B);
 SPI.begin(SCK_PIN,MISO_PIN,MOSI_PIN,-1);
 tft.initR(INITR_144GREENTAB);tft.setRotation(0);
 Serial.println(String("[SYSTEM] MakerVault v")+MAKERVAULT_FIRMWARE_VERSION+" / shared hardware SPI + Wi-Fi provisioning");
 splash();mvLoad();provisioningBegin();if(setupAPActive){page=SETUP_WIFI;render();}else home();lastInputAt=millis();
 Serial.println("[SYSTEM] Home menu ready; reader check deferred");
 Serial.println(String("[SYSTEM] READY v")+MAKERVAULT_FIRMWARE_VERSION);
}
void loop(){
 provisioningLoop();
 mvLoop();mvHeartbeatLoop();
 regScheduleCancel();
 scanLookupDispatch();
 regSchedulePoll();
 regCheckStatus();
 regSubmissionWatchdog();
 if(regServerClosed && page==REGISTRATION){regReset();page=MAKERTAGS_MENU;render();}
 remoteLookupPoll();
#if MAKERVAULT_NDEF_WRITER_EXPERIMENTAL
 ndefSchedulePoll();
 ndefScheduleCancel();
 ndefExpireLocal();
 if(ndefState==1 && page!=NDEF_WRITE && regState==0 && remoteLookupState==0){page=NDEF_WRITE;lastPoll=0;render();}
 if(page==NDEF_WRITE){static uint8_t shown=255;if(shown!=ndefState){shown=ndefState;ndefWriterScreen();}}
#endif
 if(remoteLookupState==1 && page!=LOOKUP_REQUEST && regState==0){page=LOOKUP_REQUEST;lastPoll=0;render();}
 if(page==LOOKUP_REQUEST){static uint8_t lastRemoteState=255;if(lastRemoteState!=remoteLookupState){lastRemoteState=remoteLookupState;remoteLookupScreen();}}
 if(regState==1 && page!=REGISTRATION){scanResultVisible=false;lastPoll=0;page=REGISTRATION;render();}
 if(page==REGISTRATION && regState==1){static uint32_t lastNamePage=UINT32_MAX;uint32_t currentNamePage=millis()/4000UL;if(currentNamePage!=lastNamePage){lastNamePage=currentNamePage;registrationScreen();}}
 if(page==REGISTRATION && (regState==4||regState==5||regState==6)){static uint8_t lastDrawn=0;static uint32_t lastReassignPage=UINT32_MAX;uint32_t reassignPage=millis()/4000UL;if(lastDrawn!=regState || (regState==6 && lastReassignPage!=reassignPage)){lastDrawn=regState;lastReassignPage=reassignPage;registrationScreen();}}
 processInputs();
 if(!readerInitDone && (uint32_t)(millis()-lastInputAt)>1100)readerInit();
 if(readerInitDone)pollTag();
 if(page==SCAN && scanResultVisible){static uint8_t lastScanLookup=255;static uint32_t lastScanPaint=0;if(lastScanLookup!=scanLookupState || (scanLookupState==2 && millis()-lastScanPaint>4000)){lastScanLookup=scanLookupState;lastScanPaint=millis();scanPage();}}
 if((uint32_t)(millis()-footerChecked)>250){footerChecked=millis();refreshFooter();
  if(page==MV_PAIR && mvPairState==2){static uint32_t prevSecond=0;uint32_t second=(millis()-mvPairStarted)/1000;if(second!=prevSecond){prevSecond=second;mvPairPage();}}
  if(page==MV_PAIR){
   static uint8_t drawnState=255,drawnCheck=255;static uint32_t pairedAt=0;
   if(drawnState!=mvPairState || (mvPairState==3 && drawnCheck!=mvCheckState)){
    drawnState=mvPairState;drawnCheck=mvCheckState;mvPairPage();
    pairedAt=(mvPairState==3&&mvCheckState==3)?millis():0;
   }
   if(pairedAt && (uint32_t)(millis()-pairedAt)>2300){pairedAt=0;page=MV_MENU;render();}
  }
 }
 if(page==MV_TLS_TEST){
  static uint32_t lastTestDrawn=0;
  if(lastTestDrawn!=mvTestCompleted){lastTestDrawn=mvTestCompleted;detailScroll=0;mvTlsTestPage();}
 }
 if(page==SETUP_WIFI && !setupAPActive){page=WIFI_MENU;render();}
 if(page==MAKERVAULT_SETUP && !setupPortalRunning()){page=MV_MENU;render();}
}
