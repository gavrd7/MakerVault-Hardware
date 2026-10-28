#pragma once
// Phase 1 browser-initiated Maker Tag handoff. No physical tag memory writes.
// Reuses the paired MakerVault HTTPS client, scoped credential and task slot.
String regId="", regTarget="", regRecordCode="", regUid="", regKind="", regError="", regSuppressedId="", regReassignToken="", regPreviousLabel="";
volatile uint8_t regState=0; // 0 idle; 1 request ready; 2 tag scanned; 3 sending; 4 success; 5 error; 6 reassign confirmation
uint32_t regLastCheck=0;

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
  if(status==201||status==200) {regState=4;regError="";}
  else {
    if(status==409 && response.indexOf("\"reassign_required\": true")>=0 ||
       status==409 && response.indexOf("\"reassign_required\":true")>=0) {
      regReassignToken=mvField(response,"reassign_token");
      regPreviousLabel=mvField(response,"current_label");
      if(regReassignToken.length()) {regState=6;mvTask=nullptr;vTaskDelete(nullptr);return;}
    }
    if(status==409)regError="Tag already in use";
    else if(status==410)regError="Request expired";
    else if(status==403||status==401)regError="Access denied";
    else regError="Registration failed";
    regState=5;
  }
  mvTask=nullptr;vTaskDelete(nullptr);
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
  mvHttps("/api/device/v1/tag-registration/"+regCancelId+"/cancel/","POST","{}",response,true);
  regCancelId="";
  mvTask=nullptr;vTaskDelete(nullptr);
}
void regReset();
void regCancel() {
  if(regState!=0 && regState!=4 && regId.length()==36 && !mvTask && mvEndpointApproved()) {
    regCancelId=regId;
    if(xTaskCreatePinnedToCore(regCancelTask,"mv_reg_cancel",12288,nullptr,1,&mvTask,0)!=pdPASS)
      mvTask=nullptr;
  }
  regReset();
}
void regReset() {
  regSuppressedId=regId;
  regState=0;regId="";regUid="";regKind="";regTarget="";regRecordCode="";regError="";regReassignToken="";regPreviousLabel="";regLastCheck=millis();
}
