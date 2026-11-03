#pragma once
// MakerTag physical NFC clean / optional server registration removal.
// No tag writing occurs until the exact request is re-authorized over HTTPS.
// Uses the same shared TLS task slot and RC522 page adapter as the NFC writer.
#include "ntag215_clean.h"
#include "ntag215_rc522_page_io.h"

String opId="",opUid="",opMakerId="",opLabel="",opError="";
String opAction="clean",opSource="terminal",opCancelId="";
bool opAlsoClean=true;
volatile uint8_t opState=0;
// 0 idle, 1 present tag, 2 physical confirm, 3 beginning terminal request,
// 4 checking live authority, 5 clean on same tag, 6 fresh independent readback,
// 7 report clean, 8 report remove, 9 done, 10 failed, 11 choose clean on remove,
// 12 clean reported locally, server retry, 13 wipe done, removal retry,
// 14 reporting clean, 15 reporting removal, 17 recheck tag for remove-only.
uint32_t opLastPoll=0,opAuthorizedAt=0,opLastWatch=0;

bool opJsonClean(const String &body){
  return body.indexOf("\"also_clean\": true")>=0 ||
         body.indexOf("\"also_clean\":true")>=0;
}
bool opConnected() {
  return mvPairVerified && mvCanWriteTags && mvEndpointApproved() &&
    mvCheckState==3 && WiFi.status()==WL_CONNECTED;
}
void opReset(){
  opId="";opUid="";opMakerId="";opLabel="";opError="";
  opAction="clean";opSource="terminal";opAlsoClean=true;
  opState=0;opAuthorizedAt=0;
}
String opEndpoint(const String &suffix){
  return "/api/device/v1/makertag-operation/"+opId+"/"+suffix+"/";
}
void opStart(const char *action){
  opReset();opAction=action;opAlsoClean=opAction=="clean";
  opState=opAction=="remove"?11:1;
}

void opPendingTask(void *){
  String body;
  int code=mvHttps("/api/device/v1/makertag-operation/pending/","GET","",body,true);
  if(code==200 && body.indexOf("\"operation\": null")<0 &&
     body.indexOf("\"operation\":null")<0 && opState==0){
    String id=mvField(body,"id");
    String uid=mvField(body,"uid");
    String action=mvField(body,"action");
    String maker=mvField(body,"maker_tag_id");
    if(id.length()==36 && (uid.length()==8||uid.length()==14) &&
       maker.length()==11 && (action=="clean"||action=="remove")){
      opId=id;opUid=uid;opMakerId=maker;opLabel=mvField(body,"label");
      opAction=action;opSource="browser";
      opAlsoClean=opJsonClean(body);
      opError="";opState=1;
    }
  }
  mvTask=nullptr;vTaskDelete(nullptr);
}
void opSchedulePoll(bool idlePage){
  if(!idlePage || opState!=0 || regState!=0 || remoteLookupState!=0 ||
     ndefState!=0 || mvTask || !opConnected())return;
  if((uint32_t)(millis()-opLastPoll)<5000)return;
  opLastPoll=millis();
  if(xTaskCreatePinnedToCore(opPendingTask,"mv_op_poll",12288,nullptr,1,&mvTask,0)!=pdPASS)
    mvTask=nullptr;
}

void opBeginTask(void *){
  String reply;
  String payload="{\"uid\":\""+opUid+"\",\"action\":\""+opAction+
    "\",\"also_clean\":"+(opAlsoClean?"true":"false")+"}";
  int code=mvHttps("/api/device/v1/makertag-operation/begin/","POST",payload,reply,true);
  if(code==201){
    String id=mvField(reply,"id");
    if(id.length()==36){
      opId=id;opMakerId=mvField(reply,"maker_tag_id");
      opLabel=mvField(reply,"label");opState=2;
    }else{opError="Invalid server response";opState=10;}
  }else{opError=mvField(reply,"error");if(!opError.length())opError="MakerTag not found/denied";opState=10;}
  mvTask=nullptr;vTaskDelete(nullptr);
}
bool opBeginTerminal(){
  if(opState!=1 || opUid.isEmpty() || opSource!="terminal" || mvTask || !opConnected())
    return false;
  opState=3;
  if(xTaskCreatePinnedToCore(opBeginTask,"mv_op_begin",12288,nullptr,1,&mvTask,0)!=pdPASS){
    mvTask=nullptr;opError="Cannot start request";opState=10;return false;
  }
  return true;
}

void opCheckTask(void *){
  String reply;
  int code=mvHttps(opEndpoint("status"),"GET","",reply,true);
  if(code==200 && mvField(reply,"state")=="pending" &&
     mvField(reply,"uid")==opUid && mvField(reply,"maker_tag_id")==opMakerId &&
     mvField(reply,"action")==opAction && opJsonClean(reply)==opAlsoClean){
    opAuthorizedAt=millis();opState=opAlsoClean?5:17;
  }else{opError="Request cancelled/changed";opState=10;}
  mvTask=nullptr;vTaskDelete(nullptr);
}
bool opConfirm(){
  if(opState!=2 || opId.length()!=36 || mvTask || !opConnected())return false;
  opState=4;
  if(xTaskCreatePinnedToCore(opCheckTask,"mv_op_verify",12288,nullptr,1,&mvTask,0)!=pdPASS){
    mvTask=nullptr;opError="Cannot verify request";opState=10;return false;
  }
  return true;
}

// Keep the terminal display in sync with browser cancellation/expiry.
// A failed status call never grants a physical write or server removal.
void opWatchTask(void *){
  const String id=opId;
  String reply;
  int code=mvHttps("/api/device/v1/makertag-operation/"+id+"/status/",
                   "GET","",reply,true);
  if(opId==id && (opState==1 || opState==2)){
    if(code==200){
      if(mvField(reply,"state")!="pending" ||
         mvField(reply,"uid")!=opUid ||
         mvField(reply,"maker_tag_id")!=opMakerId ||
         mvField(reply,"action")!=opAction ||
         opJsonClean(reply)!=opAlsoClean){
        opError="Request cancelled or changed";opState=10;
      }
    }
  }
  mvTask=nullptr;vTaskDelete(nullptr);
}
void opScheduleWatch(){
  if((opState!=1 && opState!=2) || opId.length()!=36 || mvTask ||
     !opConnected())return;
  if((uint32_t)(millis()-opLastWatch)<4000UL)return;
  opLastWatch=millis();
  if(xTaskCreatePinnedToCore(opWatchTask,"mv_op_watch",12288,nullptr,1,&mvTask,0)!=pdPASS)
    mvTask=nullptr;
}

void opCleanReportTask(void *){
  String reply;
  const String data="{\"uid\":\""+opUid+"\",\"verified_empty\":true}";
  int code=mvHttps(opEndpoint("cleaned"),"POST",data,reply,true);
  String state=mvField(reply,"state");
  if(code==200 && state=="completed" && opAction=="clean")opState=9;
  else if(code==200 && state=="cleaned" && opAction=="remove")opState=8;
  else{opError="NFC cleared; report pending";opState=12;}
  mvTask=nullptr;vTaskDelete(nullptr);
}
void opRemoveTask(void *){
  String reply;
  const String data="{\"uid\":\""+opUid+"\"}";
  int code=mvHttps(opEndpoint("remove"),"POST",data,reply,true);
  if(code==200 && mvField(reply,"state")=="completed")opState=9;
  else{opError=opAlsoClean?"NFC cleared; removal pending":"Removal not confirmed";opState=opAlsoClean?13:10;}
  mvTask=nullptr;vTaskDelete(nullptr);
}
void opDispatch(){
  if(mvTask || !opConnected() || opId.length()!=36)return;
  if(opState==7){opState=14;
    if(xTaskCreatePinnedToCore(opCleanReportTask,"mv_op_cleaned",12288,nullptr,1,&mvTask,0)!=pdPASS){
      mvTask=nullptr;opState=12;opError="NFC cleared; report pending";
    }
  }else if(opState==8){opState=15;
    if(xTaskCreatePinnedToCore(opRemoveTask,"mv_op_remove",12288,nullptr,1,&mvTask,0)!=pdPASS){
      mvTask=nullptr;opState=opAlsoClean?13:10;opError="Server unavailable";
    }
  }
}
void opRetry(){
  if(opState==12)opState=7;
  else if(opState==13)opState=8;
}
void opCancelTask(void *){
  String reply;
  mvHttps("/api/device/v1/makertag-operation/"+opCancelId+"/cancel/","POST","{}",reply,true);
  mvTask=nullptr;vTaskDelete(nullptr);
}
void opCancel(){
  if(opState==5 || opState==6 || opState==7 || opState==12 || opState==13 ||
     opState==14 || opState==15)return; // uncertain physical outcome: retain
  if(opId.length()==36 && !mvTask && opState!=9 &&
     opConnected()){
    opCancelId=opId;
    if(xTaskCreatePinnedToCore(opCancelTask,"mv_op_cancel",12288,nullptr,1,&mvTask,0)!=pdPASS)
      mvTask=nullptr;
  }
  opReset();
}

void opScan(MFRC522 &reader,const String &uid){
  if(opState==1){
    if(!opUid.isEmpty() && opUid!=uid){opError="Wrong tag - use original";return;}
    if(opAlsoClean && reader.uid.size!=7){opError="NTAG215 required";return;}
    opUid=uid;opError="";
    if(opSource=="terminal")opBeginTerminal();
    else opState=2;
    return;
  }
  if(opState==17){
    if(uid!=opUid){opError="Wrong tag UID";return;}
    if(!opConnected() || (uint32_t)(millis()-opAuthorizedAt)>15000UL){
      opError="Removal authorization expired";opState=10;
    }else{opError="";opState=8;}
    return;
  }
  if(reader.uid.size!=7 || !MakerVaultNtag215::matchesUid(
     reader.uid.uidByte,reader.uid.size,opUid.c_str())){
    opError="Wrong NFC tag";return;
  }
  MakerVaultRc522PageIO io(reader);
  if(opState==5){
    if(!opConnected() || (uint32_t)(millis()-opAuthorizedAt)>15000UL){
      opError="Clean authorization expired";opState=10;return;
    }
    const auto result=MakerVaultNtag215::cleanUserData(
       io,reader.uid.uidByte,opUid.c_str(),opMakerId.c_str(),true,true);
    if(result==MakerVaultNtag215::CLEAN_VERIFIED){opError="";opState=6;}
    else{opError="NFC clean refused or incomplete";opState=10;}
  }else if(opState==6){
    uint8_t memory[MakerVaultNtag215::kUserBytes]={};
    if(!MakerVaultNtag215::readNdefArea(io,memory) ||
       !MakerVaultNtag215::emptyUserData(memory,sizeof(memory))){
      opError="Fresh NFC readback failed";opState=10;
    }else{opError="";opState=7;}
  }
}
