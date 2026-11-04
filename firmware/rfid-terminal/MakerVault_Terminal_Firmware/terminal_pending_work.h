#pragma once
// One idle poll for all browser-to-terminal work. No new physical authority is
// granted by this snapshot: every action retains its existing per-request
// HTTPS status check, UID binding and physical confirmation.
uint32_t pendingWorkChecked=0;

// Extract a top-level JSON object safely even when it contains nested target
// structures or escaped strings. The server returns null when no work exists.
String pendingWorkObject(const String &json,const char *name){
 const String key=String("\"")+name+"\"";
 int pos=json.indexOf(key);
 if(pos<0)return "";
 pos=json.indexOf(':',pos+key.length());
 if(pos<0)return "";
 ++pos;
 while(pos<(int)json.length() && isspace((unsigned char)json[pos]))++pos;
 if(pos>=(int)json.length() || json[pos]!='{')return "";
 const int start=pos;
 int depth=0;bool quoted=false,escaped=false;
 for(;pos<(int)json.length();++pos){
  char c=json[pos];
  if(quoted){
   if(escaped)escaped=false;
   else if(c=='\\')escaped=true;
   else if(c=='"')quoted=false;
  }else{
   if(c=='"')quoted=true;
   else if(c=='{')++depth;
   else if(c=='}' && --depth==0)return json.substring(start,pos+1);
  }
 }
 return "";
}

void pendingWorkTask(void *){
 String body;
 const int code=mvHttps("/api/device/v1/pending-work/","GET","",body,true);
 if(code==200 && regState==0 && remoteLookupState==0
#if MAKERVAULT_NDEF_WRITER_ENABLED
    && ndefState==0 && opState==0
#endif
   ){
  // If the terminal restarted after a verified NFC wipe, preserving the
  // existing operation has priority over fresh requests. Never wipe twice.
#if MAKERVAULT_NDEF_WRITER_ENABLED
  String operation=pendingWorkObject(body,"operation");
  if(operation.length()){
   String id=mvField(operation,"id"),uid=mvField(operation,"uid");
   String action=mvField(operation,"action"),maker=mvField(operation,"maker_tag_id");
   if(id.length()==36 && (uid.length()==8||uid.length()==14) &&
      maker.length()==11 && (action=="clean"||action=="remove")){
    opId=id;opUid=uid;opMakerId=maker;opLabel=mvField(operation,"label");
    opAction=action;opSource="browser";opAlsoClean=opJsonClean(operation);
    if(mvField(operation,"state")=="cleaned" &&
       action=="remove" && opAlsoClean){
     opError="NFC cleared; removal pending";opState=13;
    }else{
     opError="";opState=1;
    }
   }
  }
  if(opState==0){
#endif
   String registration=pendingWorkObject(body,"registration");
   if(registration.length()){
    String found=mvField(registration,"id");
    if(found.length()==36 && found!=regSuppressedId){
     regId=found;regTarget=mvField(registration,"label");
     regRecordCode=mvField(registration,"record_code");
     if(regTarget.isEmpty())regTarget="MakerVault record";
     regState=1;
    }
   }
#if MAKERVAULT_NDEF_WRITER_ENABLED
  }
  if(opState==0 && regState==0){
   String write=pendingWorkObject(body,"ndef_write");
   const String found=mvField(write,"id"),uid=mvField(write,"uid");
   const String tag=mvField(write,"maker_tag_id"),url=mvField(write,"url");
   if(found.length()==36 && uid.length()==14 && tag.length()==11 &&
      url.length()>=20 && url.length()<=240 &&
      MakerVaultNtag215::matchesUid(nullptr,0,uid.c_str())==false &&
      MakerVaultNtag215::hasIdentitySuffix(url.c_str(),tag.c_str())){
    ndefId=found;ndefUid=uid;ndefTagId=tag;ndefUrl=url;
    ndefError="";ndefExistingUrl="";ndefStarted=millis();ndefState=1;
   }
  }
#endif
  if(regState==0 && remoteLookupState==0
#if MAKERVAULT_NDEF_WRITER_ENABLED
      && ndefState==0 && opState==0
#endif
     ){
   String lookup=pendingWorkObject(body,"lookup");
   String id=mvField(lookup,"id");
   if(id.length()==36 && id!=remoteLookupDismissed){
    remoteLookupId=id;remoteLookupState=1;
   }
  }
 }
 mvTask=nullptr;vTaskDelete(nullptr);
}

void pendingWorkPoll(bool idlePage){
 if(!idlePage || mvTask || regState!=0 || remoteLookupState!=0 ||
    !mvPairVerified || !mvEndpointApproved() ||
    !mvCanReadTags || mvCheckState!=3 || WiFi.status()!=WL_CONNECTED)return;
#if MAKERVAULT_NDEF_WRITER_ENABLED
 if(ndefState!=0 || opState!=0)return;
#endif
 if((uint32_t)(millis()-pendingWorkChecked)<5000UL)return;
 pendingWorkChecked=millis();
 if(xTaskCreatePinnedToCore(pendingWorkTask,"mv_pending",12288,nullptr,1,&mvTask,0)!=pdPASS)
   mvTask=nullptr;
}
