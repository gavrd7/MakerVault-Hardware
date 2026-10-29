#pragma once
// Browser-approved NTAG215 NDEF write handoff. Separate from normal scanning,
// registration and CFS code. Feature is disabled until physical acceptance.
#include "ntag215_feature_gate.h"
#include "ntag215_rc522_page_io.h"

String ndefId="", ndefUid="", ndefTagId="", ndefUrl="", ndefError="", ndefCancelId="";
volatile uint8_t ndefState=0;
// 0 idle; 1 present tag; 2 physical confirm; 3 checking live server;
// 4 authorized physical write; 5 reporting verification; 6 success; 7 error
uint32_t ndefLastPoll=0, ndefStarted=0, ndefAuthorizedAt=0;
volatile bool ndefCancelledFromBrowser=false;

void ndefPendingTask(void *) {
  String body;
  int status=mvHttps("/api/device/v1/ndef-write/pending/","GET","",body,true);
  if(status==200 && body.indexOf("\"request\": null")<0 &&
     body.indexOf("\"request\":null")<0) {
    const String found=mvField(body,"id");
    const String uid=mvField(body,"uid");
    const String tag=mvField(body,"maker_tag_id");
    const String url=mvField(body,"url");
    if(found.length()==36 && uid.length()==14 && tag.length()==11 &&
       url.length()>=20 && url.length()<=240 &&
       MakerVaultNtag215::matchesUid(nullptr,0,uid.c_str())==false &&
       MakerVaultNtag215::hasIdentitySuffix(url.c_str(),tag.c_str())) {
      ndefId=found;ndefUid=uid;ndefTagId=tag;ndefUrl=url;
      ndefError="";ndefStarted=millis();ndefState=1;
    }
  }
  mvTask=nullptr;vTaskDelete(nullptr);
}

void ndefSchedulePoll() {
  if(!MAKERVAULT_NDEF_WRITER_EXPERIMENTAL || ndefState!=0 || regState!=0 ||
     remoteLookupState!=0 || mvTask || !mvPairVerified || !mvCanWriteTags ||
     !mvEndpointApproved() || mvCheckState!=3 || WiFi.status()!=WL_CONNECTED)
    return;
  if((uint32_t)(millis()-ndefLastPoll)<5000)return;
  ndefLastPoll=millis();
  if(xTaskCreatePinnedToCore(ndefPendingTask,"mv_ndef_poll",12288,nullptr,1,&mvTask,0)!=pdPASS)
    mvTask=nullptr;
}

void ndefStatusTask(void *) {
  String reply;
  int status=mvHttps("/api/device/v1/ndef-write/"+ndefId+"/status/","GET","",reply,true);
  if(status==200 && mvField(reply,"state")=="pending" &&
     mvField(reply,"uid")==ndefUid && mvField(reply,"url")==ndefUrl &&
     mvField(reply,"maker_tag_id")==ndefTagId)
    {ndefAuthorizedAt=millis();ndefState=4;}
  else {
    ndefError="Request cancelled or changed";
    ndefState=7;
  }
  mvTask=nullptr;vTaskDelete(nullptr);
}

bool ndefConfirm() {
  if(ndefState!=2 || mvTask || !mvPairVerified || !mvCanWriteTags ||
     !mvEndpointApproved() || WiFi.status()!=WL_CONNECTED) return false;
  ndefState=3;
  if(xTaskCreatePinnedToCore(ndefStatusTask,"mv_ndef_check",12288,nullptr,1,&mvTask,0)!=pdPASS){
    ndefError="Cannot check request";ndefState=7;mvTask=nullptr;return false;
  }
  return true;
}

void ndefReportTask(void *) {
  String reply;
  const String body="{\"uid\":\""+ndefUid+"\",\"verified_url\":\""+ndefUrl+"\"}";
  const int status=mvHttps("/api/device/v1/ndef-write/"+ndefId+"/complete/","POST",body,reply,true);
  if(status==200 && mvField(reply,"state")=="completed"){
    ndefError="";ndefState=6;
  } else {
    ndefError="Tag written; server confirmation failed";
    ndefState=7;
  }
  mvTask=nullptr;vTaskDelete(nullptr);
}

bool ndefSubmitVerified() {
  if(ndefState!=5 || mvTask || !mvPairVerified || !mvCanWriteTags ||
     !mvEndpointApproved() || WiFi.status()!=WL_CONNECTED) {
    ndefError="Tag written; connection unavailable";ndefState=7;return false;
  }
  if(xTaskCreatePinnedToCore(ndefReportTask,"mv_ndef_result",12288,nullptr,1,&mvTask,0)!=pdPASS){
    ndefError="Tag written; result not sent";ndefState=7;mvTask=nullptr;return false;
  }
  return true;
}

void ndefCancelTask(void *) {
  const String cancelledId=ndefCancelId;
  String response;
  mvHttps("/api/device/v1/ndef-write/"+cancelledId+"/cancel/","POST","{}",response,true);
  if(ndefCancelId==cancelledId)ndefCancelId="";
  mvTask=nullptr;vTaskDelete(nullptr);
}

void ndefScheduleCancel() {
  if(ndefCancelId.length()!=36 || mvTask || !mvPairVerified ||
     !mvEndpointApproved() || WiFi.status()!=WL_CONNECTED)return;
  if(xTaskCreatePinnedToCore(ndefCancelTask,"mv_ndef_cancel",12288,nullptr,1,&mvTask,0)!=pdPASS)
    mvTask=nullptr;
}

void ndefReset(bool cancelServer) {
  if(cancelServer && ndefId.length()==36 && ndefState!=6)
    ndefCancelId=ndefId;
  ndefState=0;ndefId="";ndefUid="";ndefTagId="";ndefUrl="";
  ndefError="";ndefStarted=0;ndefAuthorizedAt=0;ndefLastPoll=millis();
}

// Call from loop, never from an HTTPS task. Authorization is short lived,
// including when the tag is removed while awaiting the next presentation.
void ndefExpireLocal(){
 if(ndefState>=1 && ndefState<=5 &&
    ((uint32_t)(millis()-ndefStarted)>270000UL ||
     (ndefState==4 && (uint32_t)(millis()-ndefAuthorizedAt)>15000UL) ||
     !mvPairVerified || !mvEndpointApproved())){
  ndefError="Request expired; start again";ndefState=7;
 }
}
