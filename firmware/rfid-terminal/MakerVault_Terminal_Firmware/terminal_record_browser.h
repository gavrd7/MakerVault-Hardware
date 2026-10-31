#pragma once
// Terminal-initiated, read-only MakerTag target browser (development).
// No tag registration, reassignment or NFC writes originate here.
static const char* const RB_TYPES[7] = {
 "inventory", "location", "printed_part", "printer", "project", "reusable_spool", "spool"
};
static const char* const RB_NAMES[7] = {
 "Inventory", "Location", "Printed Part", "Printer", "Project", "Reusable Spool", "Spool"
};
static const char* const RB_LETTERS = "#ABCDEFGHIJKLMNOPQRSTUVWXYZ";
constexpr int RB_PAGE_SIZE = 8;
struct RbRow {String id, label, code;};
RbRow rbRows[RB_PAGE_SIZE];
String rbLetter="", rbError="";
String rbChosenId="", rbChosenName="", rbChosenCode="";
volatile uint8_t rbStatus=0; // 0 idle, 1 queued, 2 loading, 3 ready, 4 error
volatile uint32_t rbEpoch=0;
uint8_t rbCategory=0, rbRow=0, rbLetterIndex=1, rbCount=0;
int rbOffset=0;
bool rbMore=false, rbSelectLast=false;

// Decode only the flat, bounded fields of a record. Reject incomplete JSON.
String rbJsonField(const String& source, const char* field) {
 const String key=String("\"")+field+"\"";
 int i=source.indexOf(key);
 if(i<0)return "";
 i=source.indexOf(':', i+key.length());
 if(i<0)return "";
 while(++i<(int)source.length() && isspace((unsigned char)source[i])){}
 if(i>=(int)source.length() || source[i]!='"')return "";
 String result;result.reserve(70);
 for(++i;i<(int)source.length();++i) {
  char c=source[i];
  if(c=='"')return result;
  if((unsigned char)c<32)return "";
  if(c=='\\') {
   if(++i>=(int)source.length())return "";
   c=source[i];
   if(c=='u') {
    if(i+4>=(int)source.length())return "";
    // The standard 5x7 TFT font is ASCII. Unrepresentable text gets
    // a visible placeholder instead of exposing JSON escape sequences.
    result+='?';i+=4;continue;
   }
   if(c=='n'||c=='r'||c=='t'){result+=' ';continue;}
   if(c!='"'&&c!='\\'&&c!='/')return "";
  }
  if(result.length()<100)result+=c;
 }
 return "";
}
bool rbDecode(const String& json, RbRow* rows, uint8_t &size, bool &more) {
 const int start=json.indexOf("\"rows\"");
 if(start<0)return false;
 int i=json.indexOf('[',start+6);
 if(i<0)return false;
 size=0;
 while(++i<(int)json.length()) {
  while(i<(int)json.length() && isspace((unsigned char)json[i]))i++;
  if(i>=(int)json.length())return false;
  if(json[i]==']')break;
  if(json[i]==',')continue;
  if(json[i]!='{'||size>=RB_PAGE_SIZE)return false;
  int end=i+1;
  bool inString=false,escaped=false;
  for(;end<(int)json.length();++end){
   char c=json[end];
   if(escaped){escaped=false;continue;}
   if(c=='\\'&&inString){escaped=true;continue;}
   if(c=='"'){inString=!inString;continue;}
   if(c=='}'&&!inString)break;
  }
  if(end>=(int)json.length())return false;
  const String row=json.substring(i,end+1);
  rows[size].id=rbJsonField(row,"id");
  rows[size].label=rbJsonField(row,"label");
  rows[size].code=rbJsonField(row,"record_code");
  if(rows[size].id.length()!=36 || rows[size].label.isEmpty())return false;
  ++size;i=end+1;
  // next iteration starts after object
  --i;
 }
 const int moreKey=json.indexOf("\"has_more\"");
 if(moreKey<0)return false;
 const int colon=json.indexOf(':',moreKey+10);
 if(colon<0)return false;
 String tail=json.substring(colon+1);
 tail.trim();
 if(tail.startsWith("true"))more=true;
 else if(tail.startsWith("false"))more=false;
 else return false;
 return true;
}
void rbQueue(int offset, const String& letter, bool selectLast=false){
 rbEpoch++;
 rbOffset=offset;rbLetter=letter;rbSelectLast=selectLast;rbError="";
 rbCount=0;rbRow=0;rbMore=false;rbStatus=1;
}
void rbTask(void*) {
 const uint32_t epoch=rbEpoch;
 const uint8_t category=rbCategory;
 const int offset=rbOffset;
 const String letter=rbLetter;
 String response;
 String path=String("/api/device/v1/records/browse/?type=")+RB_TYPES[category]
             +"&limit="+String(RB_PAGE_SIZE)+"&offset="+String(offset);
 if(letter.length())path+="&letter="+(letter=="#"?"%23":letter);
 int code=mvHttps(path,"GET","",response,true);
 RbRow parsed[RB_PAGE_SIZE];uint8_t count=0;bool more=false;
 bool ok=code==200 && rbDecode(response,parsed,count,more);
 if(epoch==rbEpoch) {
  if(ok) {
   for(uint8_t j=0;j<count;j++)rbRows[j]=parsed[j];
   rbCount=count;rbMore=more;
   rbRow=rbSelectLast&&count?count-1:0;
   rbStatus=3;
  }else{
   rbError=code==401?"Pair terminal":code==403?"Permission denied":
       code==-1002?"Results too large":"Cannot load records";
   rbStatus=4;
  }
 }
 mvTask=nullptr;vTaskDelete(nullptr);
}
void rbDispatch(){
 if(rbStatus!=1 || mvTask)return;
 if(!mvPairVerified || !mvCanSearchInventory || !mvEndpointApproved() ||
    WiFi.status()!=WL_CONNECTED){
  rbError="Connect / grant access";rbStatus=4;return;
 }
 rbStatus=2;
 if(xTaskCreatePinnedToCore(rbTask,"mv_records",12288,nullptr,1,&mvTask,0)!=pdPASS){
  mvTask=nullptr;rbError="Memory unavailable";rbStatus=4;
 }
}
