#include "../firmware/rfid-terminal/MakerVault_Terminal_Firmware/ntag215_clean.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

using namespace MakerVaultNtag215;
struct Memory : PageIO {
  uint8_t pages[133][4]={};
  unsigned writes=0;
  bool rejectWrite=false, corruptAfterWrite=false;
  Memory() {
    pages[3][0]=0xE1;pages[3][1]=0x10;pages[3][2]=0x3E;pages[3][3]=0;
    pages[131][3]=0xFF;
  }
  bool readPage(uint8_t page,uint8_t out[4]) override {
    memcpy(out,pages[page],4);
    if (corruptAfterWrite && writes && page==5) out[0]^=0xFF;
    return true;
  }
  bool writePage(uint8_t page,const uint8_t bytes[4]) override {
    assert(page>=4 && page<=127);
    if (rejectWrite) return false;
    memcpy(pages[page],bytes,4);++writes;return true;
  }
};
int main(){
  const uint8_t uid[7]={0x04,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};
  const char *id="MV-A3F92C10",*uidString="04AABBCCDDEEFF";
  const char *url="https://maker.local:8443/t/MV-A3F92C10/";
  Memory valid;
  assert(writeNdef(valid,uid,uidString,id,url,true,true)==RESULT_VERIFIED);
  valid.writes=0;
  assert(cleanUserData(valid,uid,uidString,id,true,true)==CLEAN_VERIFIED);
  assert(valid.writes==124);
  uint8_t data[kUserBytes]={};
  assert(readNdefArea(valid,data) && emptyUserData(data,sizeof(data)));
  assert(valid.pages[3][0]==0xE1 && valid.pages[131][3]==0xFF);
  // Already blank remains safe to clean and physically verify.
  assert(cleanUserData(valid,uid,uidString,id,true,true)==CLEAN_VERIFIED);
  Memory unconfirmed;
  assert(cleanUserData(unconfirmed,uid,uidString,id,false,true)==CLEAN_REJECTED);
  assert(unconfirmed.writes==0);
  assert(cleanUserData(unconfirmed,uid,uidString,id,true,false)==CLEAN_REJECTED);
  assert(unconfirmed.writes==0);
  assert(cleanUserData(unconfirmed,uid,"04AABBCCDDEE00",id,true,true)==CLEAN_REJECTED);
  assert(unconfirmed.writes==0);
  Memory locked;
  locked.pages[2][2]=1;
  assert(cleanUserData(locked,uid,uidString,id,true,true)==CLEAN_REJECTED);
  assert(locked.writes==0);
  Memory protectedTag;
  protectedTag.pages[131][3]=0x08;
  assert(cleanUserData(protectedTag,uid,uidString,id,true,true)==CLEAN_REJECTED);
  assert(protectedTag.writes==0);
  Memory foreign;
  assert(writeNdef(foreign,uid,uidString,id,url,true,true)==RESULT_VERIFIED);
  foreign.writes=0;
  // Same target URL is OK; the old content must belong to this MakerTag.
  foreign.pages[5][0]='X';
  assert(cleanUserData(foreign,uid,uidString,id,true,true)==CLEAN_REJECTED);
  assert(foreign.writes==0);
  Memory failed;
  assert(writeNdef(failed,uid,uidString,id,url,true,true)==RESULT_VERIFIED);
  failed.rejectWrite=true;
  assert(cleanUserData(failed,uid,uidString,id,true,true)==CLEAN_WRITE_FAILED);
  Memory corrupt;
  assert(writeNdef(corrupt,uid,uidString,id,url,true,true)==RESULT_VERIFIED);
  corrupt.writes=0;corrupt.corruptAfterWrite=true;
  assert(cleanUserData(corrupt,uid,uidString,id,true,true)==CLEAN_VERIFY_FAILED);
  puts("NTAG215 read-only validation and physical clean tests OK");
}
