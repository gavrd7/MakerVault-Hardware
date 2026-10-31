#pragma once
#include "firmware_version.h"
#include "ntag215_feature_gate.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <time.h>
#include <esp_system.h>
#include <mbedtls/x509.h>
#include <esp32_cert_bundle.h>  // Install ESP32CertBundle (Arduino-ESP32 v3.x)

// HTTPS only, with user-provided CA certificate; never skips TLS verification.
Preferences mvPrefs;
String mvBaseUrl, mvApiKey, mvCaPem, mvCredentialOrigin;
// Bound endpoint for saved bearer credential. Different URLs fail closed.
bool mvEndpointApproved(){return !mvApiKey.isEmpty() && !mvCredentialOrigin.isEmpty() && mvBaseUrl==mvCredentialOrigin;}
String mvDeviceUid, mvPairId, mvPairCode, mvPairSecret, mvPairError, mvDeviceId, mvDeviceName;
volatile bool mvPairVerified=false;
// Scopes are refreshed only after a verified /me/ response, never inferred from
// a saved token. An absent or failed check fails closed.
volatile bool mvCanReadTags=false, mvCanWriteTags=false, mvCanSearchInventory=false, mvCanHeartbeat=false;
void mvClearScopes(){mvCanReadTags=false;mvCanWriteTags=false;mvCanSearchInventory=false;mvCanHeartbeat=false;}
bool mvScopeInMe(const String &json,const char *scope){
 int i=json.indexOf("\"scopes\"");if(i<0)return false;
 i=json.indexOf('[',i+8);if(i<0)return false;
 int end=json.indexOf(']',i+1);if(end<0)return false;
 String needle=String("\"")+scope+"\"";
 // Search only the scopes array; exact quoted token matching.
 return json.substring(i+1,end).indexOf(needle)>=0;
}
void mvUpdateScopes(const String &json){
 mvCanReadTags=mvScopeInMe(json,"tags:read");
 mvCanWriteTags=mvScopeInMe(json,"tags:write");
 mvCanSearchInventory=mvScopeInMe(json,"inventory:read");
 mvCanHeartbeat=mvScopeInMe(json,"devices:status");
}

volatile uint8_t mvPairState=0; // 0 idle,1 starting,2 pending,3 paired,4 failed,5 expired
uint32_t mvPairStarted=0,mvPairLastAttempt=0,mvHeartbeatAt=0;
bool mvPairRequested=false,mvPairCancelled=false,mvLegacyKey=false;
int mvLastHttpCode=0;
// Volatile display status is not a replacement for stored pairing credentials.
String mvLastConnectionError="Not tested", mvTlsDetail="", mvLastGoodConnection="Never";
volatile uint32_t mvTestCompleted=0;
volatile bool mvTestRunning=false;
volatile int mvTestHttpCode=0;
volatile uint8_t mvTestTlsStatus=0;
String mvTestError="Not tested", mvTestDetail="", mvTestTrust="";
// Dedicated snapshot of the most recent manual test; background checks never update it.

uint8_t mvTlsStatus=0; // 0 unknown, 1 verified, 2 rejected, 3 transport failure
String mvTrustMode(){return mvCaPem.isEmpty()?"Public CA bundle":"Custom CA";}
String mvTlsErrorText(WiFiClientSecure &client,int httpCode){
 char detail[192]={0};int tlsErr=client.lastError(detail,sizeof(detail));
 mvTlsDetail=String("HTTPClient ")+String(httpCode)+" ("+HTTPClient::errorToString(httpCode)+") / TLS "+String(tlsErr)+" ("+String(detail)+")";
 Serial.printf("[TLS] %s\n",mvTlsDetail.c_str());
 // Only classify a certificate issue when mbedTLS explicitly identifies one.
 if(tlsErr==MBEDTLS_ERR_X509_CERT_VERIFY_FAILED){mvTlsStatus=2;return "Certificate rejected (trust/name/date)";}
 mvTlsStatus=3;
 if(httpCode==HTTPC_ERROR_CONNECTION_REFUSED)return "Secure connection failed";
 if(httpCode==HTTPC_ERROR_READ_TIMEOUT)return "Server timeout";
 return "Secure connection failed";
}
String mvHttpLabel(int code){
 switch(code){case 200:return "API authenticated";case 401:return "Authentication failed";
 case 403:return "Permission denied";case 429:return "Rate-limited";
 case 503:return "MakerVault unavailable";default:return "HTTP "+String(code);}
}
void mvSaveHttpStatus(int code){
 mvLastHttpCode=code;
 if(code>0){mvTlsStatus=1;mvLastConnectionError=(code==200?String("None"):mvHttpLabel(code));
   if(code==200){time_t t=time(nullptr);char stamp[30]={0};
     if(t>1760000000){struct tm tmv;gmtime_r(&t,&tmv);strftime(stamp,sizeof(stamp),"%Y-%m-%d %H:%M UTC",&tmv);mvLastGoodConnection=stamp;}
     else mvLastGoodConnection="This boot";
   }
 }
}

volatile uint8_t mvCheckState=0; // 0 unconfigured, 1 waiting, 2 verifying, 3 connected, 4 failed
uint32_t mvLastCheck=0;
TaskHandle_t mvTask=nullptr;

bool mvValidUrl(const String &u);
void mvLoad(){
 if(!mvPrefs.begin("mv-server",true))return;
 mvBaseUrl=mvPrefs.getString("url","");
 mvApiKey=mvPrefs.getString("key","");
 mvDeviceUid=mvPrefs.getString("device_uid","");
 mvDeviceId=mvPrefs.getString("device_id","");
 if(mvDeviceUid.isEmpty()){
   uint8_t uidbytes[12];esp_fill_random(uidbytes,sizeof(uidbytes));
   mvDeviceUid="terminal-";for(auto b:uidbytes){if(b<16)mvDeviceUid+="0";mvDeviceUid+=String(b,HEX);}
   mvPrefs.end();if(mvPrefs.begin("mv-server",false)){mvPrefs.putString("device_uid",mvDeviceUid);mvPrefs.end();}
   if(!mvPrefs.begin("mv-server",true))return;
 }
 mvCaPem=mvPrefs.getString("ca","");
 mvCredentialOrigin=mvPrefs.getString("key_origin","");
 mvPrefs.end();
 // One-time legacy migration: bind an existing credential to its configured URL.
 // Never rebind a credential merely because its editable URL subsequently changes.
 if(!mvApiKey.isEmpty() && mvCredentialOrigin.isEmpty() && mvValidUrl(mvBaseUrl)){
   if(mvPrefs.begin("mv-server",false)){
     if(mvPrefs.putString("key_origin",mvBaseUrl)==mvBaseUrl.length())mvCredentialOrigin=mvBaseUrl;
     mvPrefs.end();
   }
 }

 mvCheckState=(mvBaseUrl.length()&&mvApiKey.length())?1:0;
 mvPairState=mvDeviceId.length()&&mvApiKey.length()?3:0;
 mvClearScopes();mvPairVerified=false; // A stored credential does not establish live pairing.
 mvLegacyKey=mvApiKey.length()&&!mvDeviceId.length();
}

// Remove only the terminal's custom CA. This never changes the server's TLS
// certificate or deletes the paired device credential/approved origin.
// Do not change trust while a HTTPS/pairing task is using mvCaPem.c_str().
bool mvRemoveCustomCA(){
 if(mvTask || mvTestRunning || mvPairState==1 || mvPairState==2)return false;
 if(!mvPrefs.begin("mv-server",false))return false;
 bool ok=mvPrefs.remove("ca") || !mvPrefs.isKey("ca");
 mvPrefs.end();
 if(!ok)return false;
 mvCaPem="";mvPairVerified=false;mvClearScopes();mvDeviceName="";
 mvCheckState=mvApiKey.isEmpty()?0:1;
 mvLastHttpCode=0;mvLastCheck=0;mvHeartbeatAt=0;
 mvTlsStatus=0;mvTlsDetail="";mvLastConnectionError="Not tested";
 mvTestTlsStatus=0;mvTestHttpCode=0;mvTestError="Not tested";
 mvTestDetail="";mvTestTrust="Public CA bundle";
 Serial.println("[TLS] Custom CA removed locally; credentials preserved; public trust enabled");
 return true;
}

// Forget only LOCAL device association, leaving Wi-Fi, server URL and CA intact.
// Server revocation is done in MakerVault Connected Devices, separately.
bool mvForgetLocalPairing(){
 if(mvTask || mvPairState==1 || mvPairState==2)return false;
 if(!mvPrefs.begin("mv-server",false))return false;
 bool a=mvPrefs.remove("key") || !mvPrefs.isKey("key");
 bool b=mvPrefs.remove("device_id") || !mvPrefs.isKey("device_id");
 bool c=mvPrefs.remove("key_origin") || !mvPrefs.isKey("key_origin");
 mvPrefs.end();
 if(!a||!b||!c)return false;
 mvCredentialOrigin="";mvApiKey="";mvDeviceId="";mvDeviceName="";mvLegacyKey=false;
 mvPairVerified=false;mvClearScopes();mvPairState=0;mvCheckState=0;mvLastHttpCode=0;
 mvHeartbeatAt=0;mvLastCheck=0;
 return true;
}
const char* mvPairingLabel(){
 if(mvApiKey.isEmpty())return "Not paired";
 if(!mvEndpointApproved())return "Saved key: other server";
 if(mvLegacyKey)return mvCheckState==3?"Manual key active":"Manual key saved";
 if(mvPairVerified && mvCheckState==3)return "Paired / verified";
 if(mvLastHttpCode==401 || mvLastHttpCode==403)return "Pairing rejected";
 return "Credential saved";
}
bool mvValidUrl(const String &u){
 if(!u.startsWith("https://") || u.length()<12 || u.length()>220)return false;
 for(size_t i=0;i<u.length();i++){char c=u[i]; if((unsigned char)c<=32 || c=='#' || c=='?' || c=='@')return false;}
 return true;
}

bool mvSave(String url,String key,String ca){
 url.trim();key.trim();ca.trim();
 while(url.endsWith("/"))url.remove(url.length()-1);
 if(!mvValidUrl(url) || key.isEmpty() || key.length()>512 || ca.length()>6000)return false;
 if(ca.length() && (ca.indexOf("-----BEGIN CERTIFICATE-----")<0 || ca.indexOf("-----END CERTIFICATE-----")<0))return false;
 if(!mvPrefs.begin("mv-server",false))return false;
 bool urlSaved=mvPrefs.putString("url",url)==url.length();
 bool keySaved=mvPrefs.putString("key",key)==key.length();
 bool originSaved=mvPrefs.putString("key_origin",url)==url.length();
 // Preferences.putString() returns zero for an empty string: that is not a failure.
 // Remove old CA trust when switching back to a publicly trusted certificate.
 bool caSaved=ca.isEmpty() ? (mvPrefs.remove("ca") || !mvPrefs.isKey("ca"))
                           : (mvPrefs.putString("ca",ca)==ca.length());
 mvPrefs.end();
 bool ok=urlSaved && keySaved && originSaved && caSaved;
 if(ok){mvBaseUrl=url;mvApiKey=key;mvCredentialOrigin=url;mvCaPem=ca;mvCheckState=1;mvLastCheck=0;mvLegacyKey=true;mvPairState=0;mvPairVerified=false;mvClearScopes();mvDeviceName="";}
 Serial.printf("[MAKERVAULT] Configuration %s (url=%d key=%d ca=%d)\n",ok?"saved":"failed",urlSaved,keySaved,caSaved);
 return ok;
}

bool mvSaveServer(String url,String ca){
 url.trim();ca.trim();while(url.endsWith("/"))url.remove(url.length()-1);
 if(!mvValidUrl(url)||ca.length()>6000||(ca.length()&&!ca.startsWith("-----BEGIN CERTIFICATE-----")))return false;
 if(!mvPrefs.begin("mv-server",false))return false;
 bool changed=(url!=mvBaseUrl);
 bool a=mvPrefs.putString("url",url)==url.length();
 bool b=ca.length()?mvPrefs.putString("ca",ca)==ca.length():(mvPrefs.remove("ca")||!mvPrefs.isKey("ca"));
 mvPrefs.end();if(a&&b){
   mvBaseUrl=url;mvCaPem=ca;mvLastCheck=0;mvHeartbeatAt=0;
   // Never clear credentials because the endpoint or trust certificate changed.
   // Invalidate live verification until a fresh verified request succeeds.
   mvPairVerified=false;mvClearScopes();mvDeviceName="";
   mvLastHttpCode=0;mvCheckState=mvApiKey.isEmpty()?0:1;
   if(changed && !mvEndpointApproved()){
     mvLastConnectionError="Saved key bound to previous server";
     Serial.println("[MAKERVAULT] URL changed: saved credential locked to approved origin");
   } else mvLastConnectionError="Not tested";
   return true;
 }return false;
}
// A lightweight parser for the fixed, ASCII-only fields of the v1 wire contract.
// Reject malformed/missing data and do not log server payloads containing credentials.
String mvField(const String &json,const char *name){
 String needle=String("\"")+name+"\"";int pos=json.indexOf(needle);
 if(pos<0)return "";pos=json.indexOf(':',pos+needle.length());if(pos<0)return "";
 pos++;while(pos<(int)json.length() && isspace((unsigned char)json[pos]))pos++;
 if(pos>=(int)json.length()||json[pos]!='"')return "";
 String val="";for(int i=pos+1;i<(int)json.length();i++){char c=json[i];if(c=='"')return val;if(c=='\\'||(unsigned char)c<32)return "";val+=c;}
 return "";
}
String mvRandomProof(){
 static const char *alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
 uint8_t raw[32];esp_fill_random(raw,sizeof(raw));String encoded;encoded.reserve(43);
 for(int i=0;i<30;i+=3){uint32_t v=(uint32_t(raw[i])<<16)|(uint32_t(raw[i+1])<<8)|raw[i+2];
  encoded+=alphabet[(v>>18)&63];encoded+=alphabet[(v>>12)&63];encoded+=alphabet[(v>>6)&63];encoded+=alphabet[v&63];}
 uint16_t tail=(uint16_t(raw[30])<<8)|raw[31];encoded+=alphabet[(tail>>10)&63];encoded+=alphabet[(tail>>4)&63];encoded+=alphabet[(tail&15)<<2];
 return encoded;
}
bool mvTimeReady(){configTime(0,0,"pool.ntp.org","time.cloudflare.com");
 for(int i=0;i<28;i++){if(time(nullptr)>1760000000)return true;vTaskDelay(pdMS_TO_TICKS(250));}
 return false;
}
int mvHttps(const String &path,const String &method,const String &body,String &response,bool bearer){
 response="";
 // Per-request diagnostics: never allow an earlier TLS failure to survive a new request.
 mvTlsDetail="";
 mvTlsStatus=0;
 mvLastConnectionError="Not tested";
 if(WiFi.status()!=WL_CONNECTED){mvTlsStatus=3;mvLastConnectionError="Wi-Fi offline";return -1000;}
 if(!mvValidUrl(mvBaseUrl)){mvTlsStatus=3;mvLastConnectionError="Invalid HTTPS URL";return -1001;}
 // Fail closed BEFORE DNS, TCP and HTTP for a bearer request on a changed URL.
 // This applies to /me, heartbeats and manual Test HTTPS equally.
 if(bearer && !mvApiKey.isEmpty() && !mvEndpointApproved()){
   mvTlsStatus=0;mvLastConnectionError="Credential locked to original server";
   mvTlsDetail="Stored key remains saved; restore original URL or pair with new server";
   Serial.println("[MAKERVAULT] Bearer request blocked: server URL differs from approved origin");
   return -1004;
 }
 // Resolve the hostname before the TLS handshake so DNS failures aren't mistaken
 // for certificate failures. HTTPS connections still use the original hostname.
 String host=mvBaseUrl.substring(8);int cut=host.indexOf('/');if(cut>=0)host=host.substring(0,cut);
 int port=host.lastIndexOf(':');if(port>0)host=host.substring(0,port);
 if(host.isEmpty()){mvLastConnectionError="Invalid hostname";mvTlsStatus=3;return -1001;}
 IPAddress resolved;if(WiFi.hostByName(host.c_str(),resolved)!=1){mvLastConnectionError="Cannot resolve server";mvTlsStatus=3;mvTlsDetail="DNS lookup failed";Serial.println("[TLS] DNS resolution failed");return -1003;}
 WiFiClientSecure client;
 if(mvCaPem.length())client.setCACert(mvCaPem.c_str());else client.setCACertBundle(x509_crt_bundle,x509_crt_bundle_len);
 client.setTimeout(4);HTTPClient http;http.setTimeout(4500);
 if(!http.begin(client,mvBaseUrl+path)){mvLastConnectionError="Invalid HTTPS request";mvTlsStatus=3;return -1001;}
 http.addHeader("Accept","application/json");
 if(bearer&&mvApiKey.length())http.addHeader("Authorization","Bearer "+mvApiKey);
 int code;
 if(method=="POST"){http.addHeader("Content-Type","application/json");code=http.POST(body);}else code=http.GET();
 if(code>0){mvSaveHttpStatus(code);response=http.getString();if(response.length()>4096){response="";code=-1002;mvLastConnectionError="Response too large";mvTlsStatus=3;}}
 else mvLastConnectionError=mvTlsErrorText(client,code);
 http.end();return code;
}

void mvPairTask(void *){
 // Pairing runs on a worker core. All live UI and RC522 operations stay in loop().
 if(!mvTimeReady()){mvPairError="Clock unavailable";mvPairState=4;mvTask=nullptr;vTaskDelete(nullptr);return;}
 mvPairSecret=mvRandomProof();mvPairId="";mvPairCode="";
 String body="{\"device_uid\":\""+mvDeviceUid+"\",\"device_type\":\"rfid_terminal\",\"claim_secret\":\""+mvPairSecret+"\"}";
 String response;int code=mvHttps("/api/device/v1/pairing/start/","POST",body,response,false);
 if(code!=201){
  mvLastHttpCode=code;
  // Display only a bounded, plain error field. Do not log arbitrary JSON or credentials.
  String explanation=mvField(response,"detail");
  if(explanation.isEmpty())explanation=mvField(response,"error");
  if(explanation.isEmpty())explanation=mvField(response,"message");
  if(explanation.length()>90)explanation=explanation.substring(0,90);
  if(explanation.indexOf('\n')>=0 || explanation.indexOf('\r')>=0)explanation="";
  if(code<0){
    // A failed TLS handshake cannot return an HTTP status. Preserve the
    // actual diagnosed transport error and never erase the stored key.
    mvPairError=(mvTlsStatus==2)?"Certificate rejected":mvLastConnectionError;
    if(mvPairError.isEmpty()||mvPairError=="Not tested")mvPairError="HTTPS connection failed";
  }else{
    mvPairError="Start HTTP "+String(code)+(explanation.length()?String(": ")+explanation:String(""));
  }
  // Log status only; never print pairing proof, bearer token or raw body.
  Serial.printf("[PAIRING] Start failed: code=%d reason=%s\n", code,mvPairError.c_str());
  mvPairState=4;
 }
 else{
  mvPairId=mvField(response,"pairing_id");mvPairCode=mvField(response,"confirmation_code");
  if(mvPairId.length()!=36||mvPairCode.length()!=8){mvPairError="Invalid pairing response";mvPairState=4;}
  else{mvPairStarted=millis();mvPairState=2;Serial.println("[PAIRING] Challenge ready; approve in MakerVault");}
 }
 // Safe conservative polling: 3s; stop at five minutes and on cancellation.
 while(mvPairState==2&&!mvPairCancelled&&(uint32_t)(millis()-mvPairStarted)<300000){
  vTaskDelay(pdMS_TO_TICKS(3000));if(mvPairCancelled||WiFi.status()!=WL_CONNECTED)continue;
  body="{\"claim_secret\":\""+mvPairSecret+"\"}";
  code=mvHttps("/api/device/v1/pairing/"+mvPairId+"/complete/","POST",body,response,false);
  if(code==202)continue;
  if(code==200 && mvField(response,"status")=="paired"){
   String key=mvField(response,"api_key"),id=mvField(response,"device_id");
   if(key.isEmpty()||id.length()!=36){mvPairError="Invalid credential response";mvPairState=4;break;}
   bool stored=false;
   if(mvPrefs.begin("mv-server",false)){
    bool a=mvPrefs.putString("key",key)==key.length();bool b=mvPrefs.putString("device_id",id)==id.length();
    bool c=mvPrefs.putString("key_origin",mvBaseUrl)==mvBaseUrl.length();
    stored=a&&b&&c;mvPrefs.end();
   }
   if(!stored){mvPairError="Credential storage failed";mvPairState=4;break;}
   mvApiKey=key;mvDeviceId=id;mvCredentialOrigin=mvBaseUrl;mvLegacyKey=false;mvLastCheck=0;
   // Verify before reporting authenticated pairing success.
   String verify;int verified=mvHttps("/api/device/v1/me/","GET","",verify,true);
   mvLastHttpCode=verified;
   mvPairVerified=(verified==200 && mvField(verify,"protocol")=="makervault-device-v1");
   mvCheckState=mvPairVerified?3:4;
   if(mvPairVerified){mvDeviceName=mvField(verify,"key_name");mvUpdateScopes(verify);}else mvClearScopes();
   mvPairState=3;mvHeartbeatAt=0;Serial.printf("[PAIRING] Credential stored; verify HTTP %d\n",verified);
   break;
  }
  if(code==410){mvPairState=5;mvPairError="Code expired";break;}
  if(code==429){vTaskDelay(pdMS_TO_TICKS(9000));continue;}
  if(code>=400 && code<500){mvLastHttpCode=code;mvPairError="Pair HTTP "+String(code);mvPairState=4;break;}
  // Transient network/server issues are retried until expiry.
 }
 if(mvPairState==2){mvPairState=mvPairCancelled?0:5;mvPairError=mvPairCancelled?"":"Code expired";}
 mvPairSecret="";mvTask=nullptr;vTaskDelete(nullptr);
}
void mvStartPairing(){
 if(mvTask||WiFi.status()!=WL_CONNECTED||!mvValidUrl(mvBaseUrl))return;
 mvPairCancelled=false;mvPairCode="";mvPairError="";mvPairState=1;
 if(xTaskCreatePinnedToCore(mvPairTask,"mv_pair",16384,nullptr,1,&mvTask,0)!=pdPASS){mvPairState=4;mvPairError="Insufficient memory";mvTask=nullptr;}
}
void mvCancelPairing(){mvPairCancelled=true;}
void mvVerificationTask(void *){
 bool success=false;
 if(mvTimeReady()){
   String answer;int code=mvHttps("/api/device/v1/me/","GET","",answer,true);
   success=code==200 && mvField(answer,"protocol")=="makervault-device-v1";
   if(success){mvDeviceName=mvField(answer,"key_name");mvUpdateScopes(answer);}
   else {mvClearScopes();if(code==200)mvLastConnectionError="Unexpected API protocol";}
   mvPairVerified=success&&!mvDeviceId.isEmpty()&&!mvLegacyKey;
   mvLastHttpCode=code;
   Serial.printf("[MAKERVAULT] /me HTTPS response: %d (%s)\n",code,mvLastConnectionError.c_str());
 }else{
   mvTlsStatus=3;mvLastConnectionError="Clock unavailable";
   Serial.println("[TLS] Accurate clock required for certificate verification");
 }
 mvCheckState=success?3:4;mvLastCheck=millis();mvTask=nullptr;vTaskDelete(nullptr);
}
void mvTestHttpsTask(void *){
 // Reset the captured result before this specific manual request.
 mvTlsDetail="";
 mvTlsStatus=0;
 mvLastConnectionError="Not tested";
 int code=-1004;
 if(mvTimeReady()){
  String ignored;
  code=mvHttps("/api/device/v1/me/","GET","",ignored,true);
 }else{mvTlsStatus=3;mvLastConnectionError="Clock unavailable";}
 // Snapshot results BEFORE signaling completion; no global pairing/key mutation.
 mvTestHttpCode=code;
 mvTestTlsStatus=mvTlsStatus;
 mvTestError=(code==200?String("None"):(code>0?mvHttpLabel(code):mvLastConnectionError));
 mvTestDetail=(code==200)?String(""):mvTlsDetail;
 mvTestTrust=mvTrustMode();
 Serial.printf("[TLS TEST] code=%d trust=%s result=%s\n",code,mvTestTrust.c_str(),mvTestError.c_str());
 mvTestRunning=false;
 mvTestCompleted++;
 mvTask=nullptr;vTaskDelete(nullptr);
}
bool mvStartHttpsTest(){
 if(mvTask || mvTestRunning)return false;
 mvTestRunning=true;
 mvTestHttpCode=0;
 mvTestTlsStatus=0;
 mvTestError="Test in progress";
 mvTestDetail="";
 mvTestTrust=mvTrustMode();
 if(xTaskCreatePinnedToCore(mvTestHttpsTask,"mv_tls_test",12288,nullptr,1,&mvTask,0)!=pdPASS){
  mvTestRunning=false;mvTestError="Cannot start test";mvTestCompleted++;return false;
 }
 return true;
}

void mvRequestCheck(){
 if(mvTask || WiFi.status()!=WL_CONNECTED || mvCheckState==0 || (mvApiKey.length() && !mvEndpointApproved()))return;
 mvCheckState=2;
 if(xTaskCreatePinnedToCore(mvVerificationTask,"mv_https",12288,nullptr,1,&mvTask,0)!=pdPASS){
  mvCheckState=4;mvTask=nullptr;
 }
}

void mvLoop(){
 if(mvCheckState==0)return;
 if(mvApiKey.length() && !mvEndpointApproved()){mvCheckState=4;mvPairVerified=false;mvClearScopes();return;}
 if(WiFi.status()!=WL_CONNECTED){if(!mvTask){mvCheckState=1;mvClearScopes();}return;}
 if(mvTask)return;
 // Periodic, background verification; don't block the encoder or reader.
 // Failed HTTPS checks retry automatically without button interaction.
  const uint32_t retryMs=(mvCheckState==3)?60000UL:15000UL;
  if(mvLastCheck==0 || (uint32_t)(millis()-mvLastCheck)>retryMs)mvRequestCheck();
}

const char* mvStatusName(){
 switch(mvCheckState){case 0:return "Not configured";case 1:return "Waiting for Wi-Fi";case 2:return "Checking HTTPS";case 3:return "Connected";default:return "Check failed";}
}

// Heartbeats are issued off-thread, only for wizard-paired devices.
void mvHeartbeatTask(void *){
 String reply;int code=mvHttps("/api/device/v1/heartbeat/","POST",
  String("{\"firmware_version\":\"")+MAKERVAULT_FIRMWARE_VERSION+"\",\"capabilities\":[\"rfid\",\"nfc\",\"wifi\""+String(MAKERVAULT_NDEF_WRITER_ENABLED?",\"ntag215_ndef_write\"":"")+"],\"status\":{\"wifi\":\"connected\",\"api\":\"ok\",\"hardware\":\"ok\"}}",reply,true);
 // Always record a status for diagnosis; the server updates last_seen only on 200.
 mvLastHttpCode=code;
 Serial.printf("[MAKERVAULT] heartbeat HTTP response: %d\n",code);
 if(code==401 || code==403){mvCheckState=4;mvPairVerified=false;mvClearScopes();}
 mvTask=nullptr;vTaskDelete(nullptr);
}
void mvHeartbeatLoop(){
 if(mvPairState!=3||!mvPairVerified||!mvEndpointApproved()||mvTask||mvCheckState!=3||WiFi.status()!=WL_CONNECTED)return;
 // Zero means no heartbeat has been attempted since boot/pairing: send promptly.
 if(mvHeartbeatAt!=0 && (uint32_t)(millis()-mvHeartbeatAt)<60000)return;
 mvHeartbeatAt=millis();
 if(xTaskCreatePinnedToCore(mvHeartbeatTask,"mv_beat",12288,nullptr,1,&mvTask,0)!=pdPASS){
   Serial.println("[MAKERVAULT] Failed to schedule heartbeat task");
   mvHeartbeatAt=0;mvTask=nullptr;
 }
}
