#pragma once
// Safe first stage of the terminal-side NDEF workflow. This module is
// strictly read-only: it cannot write or acknowledge completion.
// Actual card-writing capability must not be advertised until the complete
// physical confirmation, fresh server authorization and read-back path exists.
#include "ntag215_reader.h"
#include "ntag215_write_policy.h"
String ndefRequestId="",ndefExpectedUid="",ndefMakerTagId="",ndefTargetUrl="";
String ndefFailure="";
volatile uint8_t ndefWorkflowState=0; // 0 idle, 1 scan, 2 inspected, 3 rejected
uint32_t ndefLastPoll=0,ndefRequestStarted=0;
String ndefSuppressedId="";

void ndefPendingTask(void*) {
 String response;
 int code=mvHttps("/api/device/v1/ndef-write/pending/","GET","",response,true);
 if(code==200 && ndefWorkflowState==0) {
  // Reuse the existing minimal response parser; no additional library.
  if(response.indexOf("\\\"request\\\":null")<0 &&
     response.indexOf("\\\"request\\\": null")<0) {
   String id=mvField(response,"id"),uid=mvField(response,"uid");
   String marker=mvField(response,"maker_tag_id"),url=mvField(response,"url");
   if(id.length()==36 && id!=ndefSuppressedId && uid.length()==14 &&
      MakerVaultNtag215::hasIdentitySuffix(url.c_str(),marker.c_str())) {
    ndefRequestId=id;ndefExpectedUid=uid;ndefMakerTagId=marker;
    ndefTargetUrl=url;ndefRequestStarted=millis();ndefWorkflowState=1;
   }
  }
 }
 mvTask=nullptr;vTaskDelete(nullptr);
}
void ndefWorkflowPoll() {
 if(ndefWorkflowState || regState || remoteLookupState || mvTask ||
    !mvPairVerified || !mvCanWriteTags || !mvEndpointApproved() ||
    WiFi.status()!=WL_CONNECTED || mvCheckState!=3) return;
 if((uint32_t)(millis()-ndefLastPoll)<5000) return;
 ndefLastPoll=millis();
 if(xTaskCreatePinnedToCore(ndefPendingTask,"ndef_pending",12288,
                           nullptr,1,&mvTask,0)!=pdPASS)mvTask=nullptr;
}
void ndefWorkflowReset() {
 ndefSuppressedId=ndefRequestId;
 ndefRequestId="";ndefExpectedUid="";ndefMakerTagId="";ndefTargetUrl="";
 ndefFailure="";ndefWorkflowState=0;ndefLastPoll=millis();
}
void ndefWorkflowInspectSelected(MFRC522 &reader) {
 if(ndefWorkflowState!=1)return;
 if(!MakerVaultNtag215::matchesUid(reader.uid.uidByte,reader.uid.size,
                                   ndefExpectedUid.c_str())){
  ndefFailure="Tag UID mismatch";ndefWorkflowState=3;return;
 }
 const auto snapshot=MakerVaultNtagReader::inspectSelected(reader);
 if(!snapshot.complete || !snapshot.memory.writable) {
  ndefFailure="Tag unsupported / locked";ndefWorkflowState=3;return;
 }
 ndefWorkflowState=2;
}
void ndefWorkflowTick() {
 if(ndefWorkflowState==1 && (uint32_t)(millis()-ndefRequestStarted)>290000UL){
  ndefFailure="Request timed out";ndefWorkflowState=3;
 }
}
