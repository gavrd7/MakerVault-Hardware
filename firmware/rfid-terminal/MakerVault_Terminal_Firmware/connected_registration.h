#pragma once
// Phase 1 browser-initiated MakerTag handoff. No physical tag memory writes.
// Reuses the paired MakerVault HTTPS client, scoped credential and task slot.
String regAutoWriteId="", regAutoUrl="", regAutoTagId="";
String regId="", regTarget="", regRecordCode="", regUid="", regKind="", regError="", regSuppressedId="", regReassignToken="", regPreviousLabel="";
volatile uint8_t regState=0; // 0 idle; 1 request ready; 2 scanned+ready; 3 sending; 4 success; 5 error; 6 reassignment review; 7 preflight
uint32_t regLastCheck=0,regLastStatusCheck=0,regSubmitStarted=0;
volatile bool regServerClosed=false;
volatile bool regSendQueued=false;

void regPendingTask(void *) {
  String body;
  int status=mvHttps("/api/device/v1/tag-registration/pending/","GET","",body,true);
  if(status==200) {
    String found=mvField(body,"id");
    if(found.length()==36 && found!=regSuppressedId && body.indexOf("\"registration\": null")<0 &&
       body.indexOf("\"registration\":null")<0) {
      regId=found;
      regTarget=mvField(body,"label");
      regRecordCode=mvField(body,"record_code");
      if(regTarget.isEmpty())regTarget="MakerVault record";
      regState=1;
    }
  }
  // Don't make the normal scanner or pairing fail because a poll timed out.
  mvTask=nullptr;vTaskDelete(nullptr);
}
void regCompleteTask(void *) {
  String response;
  String payload="{\"kind\":\""+regKind+"\",\"uid\":\""+regUid+"\"}";
  if(regReassignToken.length()) payload="{\"kind\":\""+regKind+"\",\"uid\":\""+regUid+"\",\"reassign_token\":\""+regReassignToken+"\"}";
  int status=mvHttps("/api/device/v1/tag-registration/"+regId+"/complete/",
                     "POST",payload,response,true);
  Serial.printf("[REGISTRATION] Complete HTTP status=%d (request response %u bytes)\n",status,(unsigned)response.length());
  if(status==201||status==200) {
    regAutoWriteId=mvField(response,"ndef_write_id");
    regAutoUrl=mvField(response,"ndef_url");
    regAutoTagId=mvField(response,"ndef_tag_id");
    if(regKind!="nfc"){
      // Generic RFID/MIFARE Classic is intentionally UID-only. An NDEF
      // grant is neither requested nor required for successful assignment.
      regAutoWriteId="";regAutoUrl="";regAutoTagId="";
      regState=4;regError="";
    }else if(regAutoWriteId.length()!=36 || regAutoTagId.length()!=11 ||
       regAutoUrl.length()<20 || regAutoUrl.length()>240){
      // Registration can succeed without an NFC grant (e.g. another pending
      // request). Never leave the device claiming success while the browser
      // waits forever for a physical write that cannot be authorised.
      Serial.printf("[REGISTRATION] Assigned, but NFC grant missing/invalid (id=%u tag=%u url=%u)\\n",
        (unsigned)regAutoWriteId.length(),(unsigned)regAutoTagId.length(),(unsigned)regAutoUrl.length());
      regAutoWriteId="";regAutoUrl="";regAutoTagId="";
      regError="Assigned; NFC unavailable";
      regState=5;
    }else{
      regState=4;regError="";
    }
  }
  else {
    if(status==409 && (response.indexOf("\"reassign_required\": true")>=0 ||
       response.indexOf("\"reassign_required\":true")>=0)) {
      regReassignToken=mvField(response,"reassign_token");
      regPreviousLabel=mvField(response,"current_label");
      if(regReassignToken.length()) {regState=6;mvTask=nullptr;vTaskDelete(nullptr);return;}
    }
    if(status==409) {
      String serverError=mvField(response,"error");
      if(serverError=="Already assigned to this item.")regError="Already assigned to this item";
      else if(serverError=="Reassignment confirmation expired or invalid.")regError="Confirmation expired";
      else if(serverError.length())regError=serverError;
      else regError="Tag conflict";
    }
    else if(status==410)regError="Request expired";
    else if(status==403||status==401)regError="Access denied";
    else regError="Registration failed";
    regState=5;
  }
  mvTask=nullptr;vTaskDelete(nullptr);
}
void regProbeTask(void *) {
  String response;
  const String payload="{\"kind\":\""+regKind+"\",\"uid\":\""+regUid+"\"}";
  const int status=mvHttps("/api/device/v1/tag-registration/"+regId+"/probe/","POST",payload,response,true);
  if(status==200){
    const bool reassignment=response.indexOf("\"reassign_required\": true")>=0 ||
                            response.indexOf("\"reassign_required\":true")>=0;
    if(reassignment){
      regReassignToken=mvField(response,"reassign_token");
      regPreviousLabel=mvField(response,"current_label");
      if(regReassignToken.length())regState=6;
      else{regError="Missing reassignment confirmation";regState=5;}
    }else regState=2;
  }else{
    regError=status==410?"Request expired":status==409?"Tag not available":"Cannot check tag";
    regState=5;
  }
  mvTask=nullptr;vTaskDelete(nullptr);
}
void regScheduleProbe() {
  if(regState!=7 || mvTask || regId.length()!=36 || !mvPairVerified ||
     !mvEndpointApproved() || WiFi.status()!=WL_CONNECTED)return;
  if(xTaskCreatePinnedToCore(regProbeTask,"mv_reg_probe",12288,nullptr,1,&mvTask,0)!=pdPASS){
    regError="Cannot start tag check";regState=5;mvTask=nullptr;
  }
}

void regStatusTask(void *) {
  String response;
  int code=mvHttps("/api/device/v1/tag-registration/"+regId+"/status/","GET","",response,true);
  if(code==200) {
    if(response.indexOf("\"state\": \"cancelled\"")>=0 || response.indexOf("\"state\":\"cancelled\"")>=0 ||
       response.indexOf("\"state\": \"expired\"")>=0 || response.indexOf("\"state\":\"expired\"")>=0)
      regServerClosed=true;
  }
  mvTask=nullptr;vTaskDelete(nullptr);
}
void regCheckStatus() {
  if((regState!=1 && regState!=2 && regState!=6) || mvTask || regId.length()!=36 ||
     !mvPairVerified || !mvEndpointApproved() || WiFi.status()!=WL_CONNECTED)return;
  if((uint32_t)(millis()-regLastStatusCheck)<3000)return;
  regLastStatusCheck=millis();
  if(xTaskCreatePinnedToCore(regStatusTask,"mv_reg_state",12288,nullptr,1,&mvTask,0)!=pdPASS)
    mvTask=nullptr;
}
void regSchedulePoll() {
  if(regState!=0||mvTask||mvCheckState!=3||!mvPairVerified||!mvCanWriteTags||
     !mvEndpointApproved()||WiFi.status()!=WL_CONNECTED)return;
  if((uint32_t)(millis()-regLastCheck)<5000)return;
  regLastCheck=millis();
  if(xTaskCreatePinnedToCore(regPendingTask,"mv_reg_poll",12288,nullptr,1,&mvTask,0)!=pdPASS)
    mvTask=nullptr;
}
void regDispatchSubmit() {
  if(!regSendQueued || regState!=3 || mvTask || regServerClosed)return;
  regSendQueued=false;
  if(xTaskCreatePinnedToCore(regCompleteTask,"mv_reg_send",12288,nullptr,1,&mvTask,0)!=pdPASS){
    regState=5;regError="Insufficient memory";mvTask=nullptr;
  }
}
bool regSubmit(const String &uid,const String &kind) {
  if((regState!=2 && regState!=6) || regId.length()!=36 ||
     !mvCanWriteTags || !mvEndpointApproved() || regServerClosed)return false;
  regUid=uid;regKind=kind;regSubmitStarted=millis();regState=3;
  // Preserve the user's one physical confirmation even if a harmless
  // status poll still owns the shared HTTPS task slot.
  regSendQueued=true;
  regDispatchSubmit();
  return true;
}
String regCancelId="";
void regCancelTask(void *) {
  String response;
  String cancelledId=regCancelId;
  mvHttps("/api/device/v1/tag-registration/"+cancelledId+"/cancel/","POST","{}",response,true);
  if(regCancelId==cancelledId)regCancelId="";
  mvTask=nullptr;vTaskDelete(nullptr);
}
void regScheduleCancel() {
  if(regCancelId.length()!=36 || mvTask || !mvPairVerified ||
     !mvEndpointApproved() || WiFi.status()!=WL_CONNECTED)return;
  if(xTaskCreatePinnedToCore(regCancelTask,"mv_reg_cancel",12288,nullptr,1,&mvTask,0)!=pdPASS)
    mvTask=nullptr;
}
void regReset();
void regCancel() {
  // Queue cancellation even if another HTTPS operation is currently using mvTask.
  if(regState!=0 && regState!=4 && regId.length()==36)
    regCancelId=regId;
  regReset();
  regScheduleCancel();
}
void regReset() {
  regSuppressedId=regId;
  regState=0;regId="";regUid="";regKind="";regTarget="";regRecordCode="";regError="";regReassignToken="";regPreviousLabel="";regServerClosed=false;regSendQueued=false;regSubmitStarted=0;regAutoWriteId="";regAutoUrl="";regAutoTagId="";regLastCheck=millis();
}

// A network operation must never strand the terminal on "Registering...".
// We do not discard or cancel the in-flight task: the server may already
// have committed the tag. Treat this as an uncertain result and ask the user
// to check the browser; the next poll can reconcile the server state.
void regSubmissionWatchdog() {
  if (regState!=3 || !regSubmitStarted) return;
  if ((uint32_t)(millis()-regSubmitStarted) < 30000UL) return;
  Serial.println("[REGISTRATION] Completion response delayed >30 s; check MakerVault for saved tag");
  regError="Response delayed; check MakerVault";
  regState=5;
}
