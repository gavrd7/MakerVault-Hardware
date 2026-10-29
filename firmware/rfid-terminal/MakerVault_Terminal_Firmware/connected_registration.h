#pragma once
// Phase 1 browser-initiated Maker Tag handoff. No physical tag memory writes.
// Reuses the paired MakerVault HTTPS client, scoped credential and task slot.
String regId="", regTarget="", regRecordCode="", regUid="", regKind="", regError="", regSuppressedId="", regReassignToken="", regPreviousLabel="";
volatile uint8_t regState=0; // 0 idle; 1 request ready; 2 tag scanned; 3 sending; 4 success; 5 error; 6 reassign confirmation
uint32_t regLastCheck=0,regLastStatusCheck=0;
volatile bool regServerClosed=false;

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
  if(status==201||status==200) {regState=4;regError="";}
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
bool regSubmit(const String &uid,const String &kind) {
  if(regState!=2 && regState!=6||regId.length()!=36||mvTask||!mvCanWriteTags||
     !mvEndpointApproved())return false;
  regUid=uid;regKind=kind;regState=3;
  if(xTaskCreatePinnedToCore(regCompleteTask,"mv_reg_send",12288,nullptr,1,&mvTask,0)!=pdPASS) {
    regState=5;regError="Insufficient memory";mvTask=nullptr;return false;
  }
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
  regState=0;regId="";regUid="";regKind="";regTarget="";regRecordCode="";regError="";regReassignToken="";regPreviousLabel="";regServerClosed=false;regLastCheck=millis();
}
