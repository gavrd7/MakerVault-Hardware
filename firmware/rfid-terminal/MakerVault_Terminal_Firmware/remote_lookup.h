#pragma once
// Browser-initiated read-only lookup. Never writes a physical tag or creates a
// Maker Tag. Shares the verified HTTPS task slot with pairing and heartbeat.
String remoteLookupId="", remoteLookupError="";
volatile uint8_t remoteLookupState=0; // 0 idle, 1 waiting for card, 2 uploading, 3 done, 4 error
uint32_t remoteLookupLastPoll=0;
String remoteLookupUid="",remoteLookupKind="",remoteLookupDismissed="";

void remoteLookupPollTask(void*) {
 String body;
 int status=mvHttps("/api/device/v1/tag-lookup/pending/","GET","",body,true);
 if(status==200 && body.indexOf("\"lookup\":null")<0 &&
    body.indexOf("\"lookup\": null")<0) {
  String id=mvField(body,"id");
  if(id.length()==36 && id!=remoteLookupDismissed){
   remoteLookupId=id;remoteLookupState=1;
  }
 }
 mvTask=nullptr;vTaskDelete(nullptr);
}
void remoteLookupSendTask(void*) {
 String response;
 String payload=String("{\"kind\":\"")+remoteLookupKind+"\",\"uid\":\""+remoteLookupUid+"\"}";
 int code=mvHttps("/api/device/v1/tag-lookup/"+remoteLookupId+"/complete/","POST",payload,response,true);
 if(code==200||code==201){remoteLookupState=3;}
 else {remoteLookupError=code==410?"Request expired":code==403?"Access denied":"Lookup failed";remoteLookupState=4;}
 mvTask=nullptr;vTaskDelete(nullptr);
}
void remoteLookupPoll() {
 if(remoteLookupState!=0||regState!=0||mvTask||!mvPairVerified||!mvCanReadTags||
    !mvEndpointApproved()||WiFi.status()!=WL_CONNECTED||mvCheckState!=3)return;
 if((uint32_t)(millis()-remoteLookupLastPoll)<5000)return;
 remoteLookupLastPoll=millis();
 if(xTaskCreatePinnedToCore(remoteLookupPollTask,"lookup_poll",12288,nullptr,1,&mvTask,0)!=pdPASS)
  mvTask=nullptr;
}
void remoteLookupSubmit(const String &uid,const String &kind) {
 if(remoteLookupState!=1||mvTask)return;
 remoteLookupUid=uid;remoteLookupKind=kind;remoteLookupState=2;
 if(xTaskCreatePinnedToCore(remoteLookupSendTask,"lookup_submit",12288,nullptr,1,&mvTask,0)!=pdPASS){
  remoteLookupState=4;remoteLookupError="Memory unavailable";mvTask=nullptr;
 }
}
void remoteLookupReset() {
 remoteLookupDismissed=remoteLookupId;
 remoteLookupId="";remoteLookupUid="";remoteLookupKind="";
 remoteLookupError="";remoteLookupState=0;remoteLookupLastPoll=millis();
}
