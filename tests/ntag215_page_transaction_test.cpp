#include "../firmware/rfid-terminal/MakerVault_Terminal_Firmware/ntag215_page_transaction.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

struct FakeTag : MakerVaultNtag215::PageIO {
  uint8_t memory[256][4] = {};
  unsigned writes = 0;
  int failWriteAt = -1;
  bool corruptReadback = false;
  bool sawCommit = false;
  FakeTag() {
    memory[3][0]=0xE1;memory[3][1]=0x10;
    memory[3][2]=0x3E;memory[3][3]=0x00;
    memory[131][3]=0xFF; // AUTH0 disabled (factory default)
  }
  bool readPage(uint8_t page, uint8_t out[4]) override {
    memcpy(out,memory[page],4);
    if (corruptReadback && sawCommit && page==5) out[0] ^= 0x01;
    return true;
  }
  bool writePage(uint8_t page, const uint8_t bytes[4]) override {
    if (failWriteAt >= 0 && int(writes)==failWriteAt) return false;
    assert(page>=4 && page<=127); // Never touch CC, manufacturer or locks!
    if(page==4 && bytes[1]!=0) sawCommit=true;
    memcpy(memory[page],bytes,4);++writes;return true;
  }
};

int main() {
  using namespace MakerVaultNtag215;
  const uint8_t uid[7]={0x04,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};
  const char *const url="http://192.168.1.125:8000/t/MV-A3F92C10/";
  const char *const id="MV-A3F92C10";
  const char *const uidText="04AABBCCDDEEFF";
  FakeTag blank;
  assert(writeNdef(blank,uid,uidText,id,url,true,true)==RESULT_VERIFIED);
  assert(blank.writes>2);
  uint8_t after[kUserBytes]={};
  assert(readNdefArea(blank,after));
  assert(verifyReadback(uid,7,uidText,url,after,sizeof(after)));
  // Rewriting the same link is allowed without changing the tag identity.
  assert(writeNdef(blank,uid,uidText,id,url,true,true)==RESULT_VERIFIED);
  // Genuine NTAG215 factory default has STRG_MOD_EN (CFG0 bit 2) set.
  // This is an RF modulation setting, not a password or permanent lock.
  FakeTag factoryDefault;
  factoryDefault.memory[131][0]=0x04;
  assert(writeNdef(factoryDefault,uid,uidText,id,url,true,true)==RESULT_VERIFIED);
  FakeTag mirrored;
  mirrored.memory[131][0]=0x44; // UID ASCII mirroring is not yet supported.
  assert(writeNdef(mirrored,uid,uidText,id,url,true,true)==RESULT_UNSAFE);
  assert(mirrored.writes==0);
  FakeTag otherCfg;
  otherCfg.memory[131][0]=0x08;
  assert(writeNdef(otherCfg,uid,uidText,id,url,true,true)==RESULT_UNSAFE);
  assert(otherCfg.writes==0);
  FakeTag notConfirmed;
  assert(writeNdef(notConfirmed,uid,uidText,id,url,false,true)==RESULT_UNSAFE);
  assert(notConfirmed.writes==0);
  FakeTag wrongUid;
  assert(writeNdef(wrongUid,uid,"04AABBCCDDEE00",id,url,true,true)==RESULT_UNSAFE);
  assert(wrongUid.writes==0);
  FakeTag locked;
  locked.memory[2][2]=1;
  assert(writeNdef(locked,uid,uidText,id,url,true,true)==RESULT_UNSAFE);
  assert(locked.writes==0);
  FakeTag protectedTag;
  protectedTag.memory[131][3]=0x05; // AUTH0 requires password from page 5
  assert(writeNdef(protectedTag,uid,uidText,id,url,true,true)==RESULT_UNSAFE);
  assert(protectedTag.writes==0);
  FakeTag accessProtected;
  accessProtected.memory[132][0]=0x80;
  assert(writeNdef(accessProtected,uid,uidText,id,url,true,true)==RESULT_UNSAFE);
  assert(accessProtected.writes==0);
  FakeTag foreign;
  foreign.memory[4][0]=0x03;foreign.memory[4][1]=0x00;foreign.memory[4][2]=0xFE;
  foreign.memory[8][1]=0x88; // Nonblank data beyond the terminator
  assert(writeNdef(foreign,uid,uidText,id,url,true,true)==RESULT_UNSAFE);
  assert(foreign.writes==0);
  FakeTag interrupted;
  interrupted.failWriteAt=1;
  assert(writeNdef(interrupted,uid,uidText,id,url,true,true)==RESULT_WRITE_FAILED);
  assert(interrupted.memory[4][1]==0x00); // No valid NDEF length committed
  FakeTag corrupt;
  corrupt.corruptReadback=true;
  assert(writeNdef(corrupt,uid,uidText,id,url,true,true)==RESULT_READBACK_MISMATCH);
  puts("MakerVault NTAG215 page transaction mock tests: OK");
}
