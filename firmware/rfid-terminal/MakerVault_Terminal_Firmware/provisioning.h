#pragma once
// MakerVault local Wi-Fi provisioning prototype v0.4.0
// Runs only on the ESP32, no external cloud connection.
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <WiFiMulti.h>
#include <new>
#include <qrcode.h>
#include "maker_backend.h"  // QRCode by Richard Moore (ricmoo)

WebServer setupServer(80);
DNSServer setupDns;
Preferences wifiPrefs;
WiFiMulti savedWiFi;
constexpr uint8_t MAX_KNOWN_NETWORKS=5;
String knownSSID[MAX_KNOWN_NETWORKS], knownPass[MAX_KNOWN_NETWORKS];
uint8_t knownCount=0;
bool reconnectRequested=false;
uint32_t lastReconnectTry=0;
void loadKnownNetworks(){
 knownCount=0;
 if(!wifiPrefs.begin("mv-wifi",true))return;
 uint8_t n=min((int)wifiPrefs.getUChar("count",0),(int)MAX_KNOWN_NETWORKS);
 for(uint8_t i=0;i<n;i++){
  String key=String("s")+i, secret=String("p")+i;
  String ssid=wifiPrefs.getString(key.c_str(),"");
  String pass=wifiPrefs.getString(secret.c_str(),"");
  if(ssid.length()&&ssid.length()<=32&&pass.length()<=63){knownSSID[knownCount]=ssid;knownPass[knownCount]=pass;knownCount++;}
 }
 // One-time migration from v0.4.0/0.4.0.1 layout.
 if(!knownCount){String ssid=wifiPrefs.getString("ssid","");String pass=wifiPrefs.getString("password","");
   if(ssid.length()&&ssid.length()<=32&&pass.length()<=63){knownSSID[0]=ssid;knownPass[0]=pass;knownCount=1;}}
 wifiPrefs.end();
}
void persistKnownNetworks(){
 if(!wifiPrefs.begin("mv-wifi",false))return;
 wifiPrefs.putUChar("count",knownCount);
 for(uint8_t i=0;i<MAX_KNOWN_NETWORKS;i++){
  String a=String("s")+i,b=String("p")+i;
  if(i<knownCount){wifiPrefs.putString(a.c_str(),knownSSID[i]);wifiPrefs.putString(b.c_str(),knownPass[i]);}
  else {wifiPrefs.remove(a.c_str());wifiPrefs.remove(b.c_str());}
 }
 // Drop obsolete single-SSID keys only after new entries committed.
 wifiPrefs.remove("ssid");wifiPrefs.remove("password");wifiPrefs.end();
}
void rebuildWiFiList(){
 // ESP32 WiFiMulti has no clear(): reconstruct the instance in place safely.
 savedWiFi.~WiFiMulti();new (&savedWiFi) WiFiMulti();
 for(uint8_t i=0;i<knownCount;i++)savedWiFi.addAP(knownSSID[i].c_str(),knownPass[i].c_str());
}
void rememberNetwork(const String &ssid,const String &pass){
 uint8_t at=knownCount;
 for(uint8_t i=0;i<knownCount;i++)if(knownSSID[i]==ssid){at=i;break;}
 if(at<knownCount){for(uint8_t i=at;i+1<knownCount;i++){knownSSID[i]=knownSSID[i+1];knownPass[i]=knownPass[i+1];}knownCount--;}
 else if(knownCount==MAX_KNOWN_NETWORKS){for(uint8_t i=0;i+1<knownCount;i++){knownSSID[i]=knownSSID[i+1];knownPass[i]=knownPass[i+1];}knownCount--;}
 knownSSID[knownCount]=ssid;knownPass[knownCount]=pass;knownCount++;
 persistKnownNetworks();rebuildWiFiList();
}
void forgetNetwork(uint8_t index){
 if(index>=knownCount)return;
 String removed=knownSSID[index];
 for(uint8_t i=index;i+1<knownCount;i++){knownSSID[i]=knownSSID[i+1];knownPass[i]=knownPass[i+1];}
 knownCount--;persistKnownNetworks();rebuildWiFiList();
 if(WiFi.SSID()==removed){WiFi.disconnect();reconnectRequested=true;}
}

bool setupAPActive=false, lanPortalActive=false, wifiJoinPending=false, wifiEverConfigured=false;
bool setupPortalRunning(){return setupAPActive || lanPortalActive;}
uint32_t joinStarted=0, portalStarted=0, connectedSince=0;
String setupSSID,setupPassword,clientSSID,clientPassword,formNonce;
const IPAddress portalIP(192,168,4,1);

String htmlEscape(const String &src){
 String s; s.reserve(src.length()+12);
 for(size_t i=0;i<src.length();i++){
  switch(src[i]){case '&':s+="&amp;";break;case '<':s+="&lt;";break;
  case '>':s+="&gt;";break;case '"':s+="&quot;";break;
  case '\'':s+="&#39;";break;default:s+=src[i];}
 }
 return s;
}
String jsonQuote(const String &src){
 String s="\"";
 for(size_t i=0;i<src.length();i++){
  char c=src[i];
  if(c=='"'||c=='\\'){s+='\\';s+=c;}
  else if((unsigned char)c<32){s+=' ';}else s+=c;
 }
 return s+'"';
}
String randomHex(uint8_t n){
 const char abc[]="0123456789ABCDEF";String s;for(int i=0;i<n;i++)s+=abc[esp_random()%16];return s;
}
void sendPortal(){
 const char* htmlHead=R"RAW(<!DOCTYPE html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>MakerVault setup</title><style>
 *{box-sizing:border-box}body{background:#071322;color:#eaf3ff;font:16px system-ui,sans-serif;margin:0;padding:26px 15px}.wrap{max-width:460px;margin:auto}.card{background:#122441;border:1px solid #24456a;border-radius:18px;padding:23px}h1{margin:0 0 6px;font-size:25px}p{color:#b7c8dc}label{display:block;margin:16px 0 6px;font-weight:600}select,input,button{font:inherit;width:100%;border-radius:9px;padding:12px}select,input{background:#07172c;color:white;border:1px solid #486280}button{background:#1173d1;color:white;border:0;font-weight:600;margin-top:18px}small{color:#b8cadb}.status{padding:11px;border-radius:8px;background:#1c3552;margin-top:12px}</style></head><body><div class="wrap"><div class="card"><h1>Maker<span style="color:#31a5ff">Vault</span></h1><p>Handheld terminal — Wi-Fi setup</p><div class="status" id="state">Scanning nearby Wi-Fi networks...</div><form action="/connect" method="post"><input type="hidden" name="nonce" value=")RAW";
 String h=htmlHead+formNonce+R"RAW("><label for="net">Nearby networks</label><select id="net" onchange="document.getElementById('ssid').value=this.value;"><option value="">Scanning...</option></select><label for="ssid">Wi-Fi name (SSID)</label><input id="ssid" name="ssid" maxlength="32" autocomplete="off" required placeholder="Choose a network or type one"><label for="password">Wi-Fi password</label><input id="password" type="password" name="password" minlength="0" maxlength="63" autocomplete="off"><button type="submit">Connect terminal</button></form><p><small>Supports 2.4 GHz Wi-Fi. Pairing with MakerVault follows after Wi-Fi setup. Never enter a MakerVault API key here.</small></p><p><a href="/makervault" style="color:#81ccff">Set up MakerVault server</a></p></div></div><script>
 async function networks(){try{let r=await fetch('/networks',{cache:'no-store'}),j=await r.json();if(j.scanning){setTimeout(networks,900);return;}let el=document.getElementById('net');el.replaceChildren(new Option('Choose a network',''));j.networks.forEach(n=>el.add(new Option(n.ssid+' ('+n.rssi+' dBm)',n.ssid)));document.getElementById('state').textContent=j.networks.length+' networks found';}catch(e){document.getElementById('state').textContent='Rescan by refreshing this page';}} networks();
 </script></body></html>)RAW";
 setupServer.sendHeader("Cache-Control","no-store");setupServer.send(200,"text/html",h);
}
void setupRoutes(){
 setupServer.on("/",HTTP_GET,sendPortal);
 setupServer.on("/makervault",HTTP_GET,[](){
  String page=R"HTML(<!doctype html><html lang="en"><meta name="viewport" content="width=device-width,initial-scale=1"><style>body{font:16px system-ui;background:#071322;color:#eaf3ff;max-width:480px;margin:30px auto;padding:12px}input,textarea,button{box-sizing:border-box;width:100%;padding:12px;margin:7px 0 17px;border-radius:8px}input,textarea{background:#122441;color:white;border:1px solid #486280}button{background:#1173d1;color:white;border:0}label{display:block}small{color:#b7c8dc}</style><h2>MakerVault server pairing</h2><p>Publicly trusted HTTPS certificates are checked automatically. Only upload a .crt/.pem file if your server uses a private CA or self-signed certificate.</p><form method="post" action="/save-maker"><input type="hidden" name="nonce" value=")HTML"+formNonce+R"HTML("><label>Server URL (https://...)</label><input name="url" type="url" required placeholder="https://vault.example.org" value=")HTML"+htmlEscape(mvBaseUrl)+R"HTML("><label>Advanced: manual API key (optional)</label><input name="key" type="password" autocomplete="off" placeholder="Paste a new device key"><label>Private CA / self-signed certificate (optional)</label><input type="file" id="certfile" accept=".crt,.pem,.cer,text/plain,application/x-pem-file"><small>For publicly trusted HTTPS leave this empty. Certificate files are read locally in your browser before form submission.</small><textarea id="pem" name="ca" rows="5" maxlength="6000" placeholder="Optional PEM certificate"></textarea><button>Save connection</button></form><script>document.getElementById("certfile").addEventListener("change",async e=>{let f=e.target.files[0];if(!f)return;if(f.size>6000){alert("Certificate file is too large (6 KB maximum)");e.target.value="";return;}let t=await f.text();if(!t.includes("-----BEGIN CERTIFICATE-----")||!t.includes("-----END CERTIFICATE-----")){alert("Select a PEM-encoded .crt or .pem certificate");return;}document.getElementById("pem").value=t;});</script><small>After saving, use Connections &gt; Pair device to display the eight-digit code for approval in MakerVault. Manual keys are optional for legacy integrations. Flash encryption is recommended for protecting stored credentials.</small><p><a href="/" style="color:#80cfff">Wi-Fi setup</a></p></html>)HTML";
  setupServer.sendHeader("Cache-Control","no-store");setupServer.send(200,"text/html",page);
 });
 setupServer.on("/save-maker",HTTP_POST,[](){
  if(setupServer.arg("nonce")!=formNonce){setupServer.send(403,"text/plain","Invalid setup session");return;}
  String url=setupServer.arg("url"),key=setupServer.arg("key"),ca=setupServer.arg("ca");
  url.trim();ca.trim();
  if(!mvValidUrl(url)||key.length()>512||ca.length()>6000||(ca.length()&&ca.indexOf("-----BEGIN CERTIFICATE-----")<0)){setupServer.send(400,"text/plain","Provide an HTTPS URL, API key and, when used, valid PEM certificate");return;}
  if(!(key.length()?mvSave(url,key,ca):mvSaveServer(url,ca))){setupServer.send(500,"text/plain","Could not save server configuration; please retry.");return;}
  setupServer.send(200,"text/html","<meta name='viewport' content='width=device-width'><body style='background:#071322;color:white;font-family:system-ui;padding:20px'><h2>Server configuration saved</h2><p>Return to the terminal and select Pair device. The terminal will show an eight-digit code to approve in MakerVault.</p></body>");
 });
 setupServer.on("/networks",HTTP_GET,[](){
  int n=WiFi.scanComplete();
  if(n==WIFI_SCAN_FAILED){WiFi.scanNetworks(true,false);n=WIFI_SCAN_RUNNING;}
  if(n==WIFI_SCAN_RUNNING){setupServer.send(200,"application/json","{\"scanning\":true,\"networks\":[]}");return;}
  String j="{\"scanning\":false,\"networks\":[";
  int count=0;
  for(int i=0;i<n && count<25;i++){
   String ssid=WiFi.SSID(i);if(ssid.isEmpty())continue;
   if(count++)j+=',';
   j+="{\"ssid\":"+jsonQuote(ssid)+",\"rssi\":"+String(WiFi.RSSI(i))+"}";
  }
  j+="]}"; WiFi.scanDelete(); setupServer.send(200,"application/json",j);
 });
 setupServer.on("/connect",HTTP_POST,[](){
  if(setupServer.arg("nonce")!=formNonce){setupServer.send(403,"text/plain","Session mismatch");return;}
  String ssid=setupServer.arg("ssid"),pass=setupServer.arg("password");
  if(ssid.length()==0||ssid.length()>32||pass.length()>63){setupServer.send(400,"text/plain","Invalid Wi-Fi settings");return;}
  clientSSID=ssid;clientPassword=pass;wifiJoinPending=true;joinStarted=millis();connectedSince=0;
  setupServer.send(200,"text/html","<meta name='viewport' content='width=device-width'><body style='font-family:sans-serif;background:#071322;color:white;padding:25px'><h2>Connecting...</h2><p>The terminal will try to join the selected network. Watch its display. If it fails, reconnect to the MakerVault setup Wi-Fi and try again.</p></body>");
  WiFi.begin(clientSSID.c_str(),clientPassword.c_str());
 });
 setupServer.onNotFound([](){setupServer.sendHeader("Location","http://192.168.4.1/",true);setupServer.send(302,"text/plain","");});
 setupServer.begin();
}
void beginPortal(){
 if(setupPortalRunning())return;
 // AP+STA permits management without disconnecting from the saved network.
 WiFi.mode(WIFI_AP_STA);
 if(!wifiPrefs.begin("mv-wifi",false)){Serial.println("[WIFI] Preferences error");return;}
 setupSSID="MakerVault-"+randomHex(4);
 setupPassword=wifiPrefs.getString("ap-secret","");
 if(setupPassword.length()!=16){setupPassword=randomHex(16);wifiPrefs.putString("ap-secret",setupPassword);}
 formNonce=randomHex(16);
 WiFi.softAPConfig(portalIP,portalIP,IPAddress(255,255,255,0));
 if(!WiFi.softAP(setupSSID.c_str(),setupPassword.c_str())){Serial.println("[WIFI] AP start failed");wifiPrefs.end();return;}
 setupDns.start(53,"*",portalIP);setupRoutes();setupAPActive=true;portalStarted=millis();
 Serial.println("[WIFI] Setup portal active at http://192.168.4.1");
 wifiPrefs.end();
}
void beginLanPortal(){
 if(setupPortalRunning())return;
 if(WiFi.status()!=WL_CONNECTED || WiFi.localIP()==IPAddress(0,0,0,0))return;
 formNonce=randomHex(16);
 setupRoutes();lanPortalActive=true;portalStarted=millis();
 Serial.print("[MAKERVAULT] Temporary LAN setup at http://");
 Serial.print(WiFi.localIP());Serial.println("/makervault");
}
void stopPortal(){
 if(!setupPortalRunning())return;
 if(setupAPActive)setupDns.stop();
 setupServer.stop();
 if(setupAPActive){WiFi.softAPdisconnect(true);WiFi.mode(WIFI_STA);}
 setupAPActive=false;lanPortalActive=false;
 Serial.println("[WIFI] Setup portal stopped");
}
void beginMakerVaultPortal(){
 if(WiFi.status()==WL_CONNECTED && WiFi.localIP()!=IPAddress(0,0,0,0)){
  if(setupAPActive)stopPortal();
  beginLanPortal();
 }else beginPortal();
}
bool wifiManualOffline=false;
void wifiSetManualOffline(bool disabled){
 wifiManualOffline=disabled;
 if(disabled && setupPortalRunning())stopPortal();
 wifiPrefs.begin("mv-wifi",false);
 wifiPrefs.putBool("manualOff",disabled);
 wifiPrefs.end();
 if(disabled){
  WiFi.setAutoReconnect(false);
  WiFi.disconnect(false,false);
  Serial.println("[WIFI] Manually disconnected; remembered networks retained");
 }else{
  WiFi.mode(WIFI_STA);WiFi.setSleep(false);WiFi.setAutoReconnect(true);
  lastReconnectTry=millis()-12001;
  if(knownCount)WiFi.begin(knownSSID[knownCount-1].c_str(),knownPass[knownCount-1].c_str());
  Serial.println("[WIFI] Automatic connection enabled");
 }
}
void provisioningBegin(){
 wifiPrefs.begin("mv-wifi",true);
 wifiManualOffline=wifiPrefs.getBool("manualOff",false);
 wifiPrefs.end();
 loadKnownNetworks();
 wifiEverConfigured=knownCount>0;
 if(knownCount && !wifiManualOffline){
  // Persist migrated credentials without clearing AP secret.
  persistKnownNetworks();rebuildWiFiList();
  WiFi.mode(WIFI_STA);WiFi.setAutoReconnect(true);
  reconnectRequested=true;lastReconnectTry=millis();
  // Begin last-known connection asynchronously; never block the menu.
  WiFi.begin(knownSSID[knownCount-1].c_str(),knownPass[knownCount-1].c_str());
 }else if(!knownCount && !wifiManualOffline)beginPortal();
 else {WiFi.mode(WIFI_STA);WiFi.setAutoReconnect(false);WiFi.disconnect(false,false);}
}
void provisioningLoop(){
 if(lanPortalActive && WiFi.status()!=WL_CONNECTED)stopPortal();
 // Try saved networks periodically without holding up button/encoder input.
 if(!wifiManualOffline && !setupAPActive && knownCount && !wifiJoinPending && WiFi.status()!=WL_CONNECTED &&
    (uint32_t)(millis()-lastReconnectTry)>12000){
  lastReconnectTry=millis();
  // WiFiMulti.run may scan synchronously, so it must not execute every loop.
  savedWiFi.run(1500);reconnectRequested=false;
 }
 if(setupAPActive)setupDns.processNextRequest();
 if(setupPortalRunning())setupServer.handleClient();
 if(wifiJoinPending){
  if(WiFi.status()==WL_CONNECTED){
   rememberNetwork(clientSSID,clientPassword);
   clientPassword="";wifiEverConfigured=true;wifiJoinPending=false;connectedSince=millis();
   Serial.println("[WIFI] Connected and settings saved");
  }else if((uint32_t)(millis()-joinStarted)>20000){
   WiFi.disconnect();wifiJoinPending=false;clientPassword="";
   Serial.println("[WIFI] Connection failed, portal remains available");
  }
 }
 if(connectedSince && setupAPActive && (uint32_t)(millis()-connectedSince)>3500){stopPortal();connectedSince=0;}
 // Setup network is time-limited after manual use, but first-run stays
 // until configured, with the user able to exit via the terminal.
 if(setupPortalRunning() && wifiEverConfigured && !wifiJoinPending && !connectedSince &&
    (uint32_t)(millis()-portalStarted)>600000)stopPortal();
}
void drawSetupQR(Adafruit_GFX &display,uint16_t bg,uint16_t fg){
 String qrPayload="WIFI:T:WPA;S:"+setupSSID+";P:"+setupPassword+";;";
 QRCode code;uint8_t qrBuffer[qrcode_getBufferSize(5)];
 qrcode_initText(&code,qrBuffer,5,ECC_LOW,qrPayload.c_str());
 display.fillScreen(bg);
 display.setTextSize(1);display.setTextColor(fg);
 display.setCursor(15,3);display.print("CONNECT TO SETUP");
 const int scale=2;const int startX=(128-(code.size+8)*scale)/2,startY=17;
 display.fillRect(startX,startY,(code.size+8)*scale,(code.size+8)*scale,0xFFFF);
 for(int y=0;y<code.size;y++)for(int x=0;x<code.size;x++){
  if(qrcode_getModule(&code,x,y))display.fillRect(startX+(x+4)*scale,startY+(y+4)*scale,scale,scale,0);
 }
 // Center both captions using the active font's actual measured width.
 display.setTextColor(fg);
 auto centeredCaption = [&](const char *caption, int16_t y) {
  int16_t bx, by; uint16_t bw, bh;
  display.getTextBounds(caption, 0, y, &bx, &by, &bw, &bh);
  display.setCursor((128 - (int16_t)bw) / 2 - bx, y);
  display.print(caption);
 };
 centeredCaption("Scan Wi-Fi QR", 108);
 centeredCaption("192.168.4.1", 119);
}
