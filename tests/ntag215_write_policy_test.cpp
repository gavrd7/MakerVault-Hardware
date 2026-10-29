#include "../firmware/rfid-terminal/MakerVault_Terminal_Firmware/ntag215_write_policy.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main() {
  const uint8_t uid[7] = {0x04,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};
  const char *const expected = "04AABBCCDDEEFF";
  const char *const id = "MV-A3F92C10";
  const char *const url = "https://vault.example.com:8443/t/MV-A3F92C10/";
  const MakerVaultNtag215::MemoryInfo clear = {true,true,496};
  const MakerVaultNtag215::MemoryInfo locked = {true,false,496};
  const MakerVaultNtag215::MemoryInfo unknown = {false,false,0};
  uint8_t blank[64] = {0};
  using namespace MakerVaultNtag215;

  assert(assess(clear,uid,7,expected,id,url,blank,sizeof(blank),true,true)==WRITE_READY);
  assert(assess(clear,uid,7,expected,id,url,blank,sizeof(blank),false,true)==WRITE_REJECT_CONFIRMATION);
  assert(assess(clear,uid,7,expected,id,url,blank,sizeof(blank),true,false)==WRITE_REJECT_CONFIRMATION);
  assert(assess(locked,uid,7,expected,id,url,blank,sizeof(blank),true,true)==WRITE_REJECT_LOCKED);
  assert(assess(unknown,uid,7,expected,id,url,blank,sizeof(blank),true,true)==WRITE_REJECT_UNSUPPORTED);
  assert(assess(clear,uid,7,"04AABBCCDDEE00",id,url,blank,sizeof(blank),true,true)==WRITE_REJECT_UID);
  assert(assess(clear,uid,7,expected,id,"https://vault.example.com/t/MV-00000000/",blank,sizeof(blank),true,true)==WRITE_REJECT_URL);
  assert(assess(clear,uid,7,expected,id,"javascript:evil",blank,sizeof(blank),true,true)==WRITE_REJECT_URL);
  blank[0]=0x03;blank[1]=0x00;blank[2]=0xFE;
  assert(assess(clear,uid,7,expected,id,url,blank,sizeof(blank),true,true)==WRITE_READY);
  uint8_t existing[MakerVaultNdef::kMaxEncodedLength] = {0};
  size_t count = 0;
  const char *oldUrl = "http://192.168.1.125:8000/t/MV-A3F92C10/";
  assert(MakerVaultNdef::encodeUri(oldUrl,existing,sizeof(existing),count));
  assert(assess(clear,uid,7,expected,id,url,existing,count,true,true)==WRITE_READY);
  assert(MakerVaultNdef::encodeUri("https://other.example/t/MV-11111111/",existing,sizeof(existing),count));
  assert(assess(clear,uid,7,expected,id,url,existing,count,true,true)==WRITE_REJECT_FOREIGN_CONTENT);
  existing[5] = 0x22;
  assert(assess(clear,uid,7,expected,id,url,existing,count,true,true)==WRITE_REJECT_FOREIGN_CONTENT);
  puts("MakerVault NTAG215 physical-write policy: OK");
}
