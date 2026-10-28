#pragma once
// Non-mutating MakerVault UID lookup for the existing normal Read tag screen.
// Uses only tags:read, verified HTTPS and the shared background task slot.
String scanLookupName="", scanLookupCode="", scanLookupError="";
volatile uint8_t scanLookupState=0; // 0 idle/offline, 1 checking, 2 found, 3 unknown, 4 error
uint32_t scanLookupSerial=0;
String scanLookupUid="", scanLookupKind="";

void scanLookupTask(void *) {
 String response;
 String body=String("{\"kind\":\"")+scanLookupKind+"\",\"uid\":\""+scanLookupUid+"\"}";
 int status=mvHttps("/api/device/v1/tags/resolve/","POST",body,response,true);
 // An older request must never overwrite a newer displayed scan.
 if(status==200 && response.indexOf("\"found\": true")>=0 || (status==200 && response.indexOf("\"found\":true")>=0)) {
   int targetPos=response.indexOf("\"target\"");
   String targetSection=targetPos>=0?response.substring(targetPos):"";
   scanLookupName=mvField(targetSection,"label");
   scanLookupCode=mvField(targetSection,"subtitle");
   if(scanLookupName.isEmpty())scanLookupName=mvField(response,"label");
   scanLookupState=2;
 } else if(status==200) {
   scanLookupState=3;
 } else {
   scanLookupError=status==403?"Access denied":status==401?"Pair again":"Lookup unavailable";
   scanLookupState=4;
 }
 mvTask=nullptr;
 vTaskDelete(nullptr);
}

void scanLookupBegin(const String &uid,const String &kind) {
 scanLookupState=0;scanLookupName="";scanLookupCode="";scanLookupError="";
 if(!mvPairVerified||!mvCanReadTags||!mvEndpointApproved()||mvTask||WiFi.status()!=WL_CONNECTED)return;
 scanLookupUid=uid;scanLookupKind=kind;
 scanLookupState=1;
 if(xTaskCreatePinnedToCore(scanLookupTask,"mv_tag_lookup",12288,nullptr,1,&mvTask,0)!=pdPASS){
   mvTask=nullptr;scanLookupState=4;scanLookupError="Memory unavailable";
 }
}
