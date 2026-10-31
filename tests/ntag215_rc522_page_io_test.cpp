#include <stdint.h>
#include <string.h>
#define MAKERVAULT_MFRC522_TEST
class MFRC522 {
 public:
  enum StatusCode { STATUS_OK, STATUS_ERROR };
  uint8_t memory[256][4] = {};
  int writes = 0;
  MFRC522() {
    memory[3][0]=0xE1;memory[3][1]=0x10;memory[3][2]=0x3E;
    memory[131][3]=0xFF;
  }
  StatusCode MIFARE_Read(uint8_t page,uint8_t *out,uint8_t *count) {
    if(!out||!count||*count<18||page>132) return STATUS_ERROR;
    for (int i=0;i<16;i++) out[i]=memory[page+i/4][i%4];
    out[16]=out[17]=0;*count=18;return STATUS_OK;
  }
  StatusCode MIFARE_Ultralight_Write(uint8_t page,uint8_t *data,uint8_t size) {
    if(!data||size!=4||page>132) return STATUS_ERROR;
    memcpy(memory[page],data,4);++writes;return STATUS_OK;
  }
};
#include "../firmware/rfid-terminal/MakerVault_Terminal_Firmware/ntag215_rc522_page_io.h"
#include <assert.h>
#include <stdio.h>

int main() {
  MFRC522 reader;
  MakerVaultRc522PageIO io(reader);
  uint8_t buf[4]={};
  assert(io.readPage(3,buf));
  assert(buf[0]==0xE1 && buf[2]==0x3E);
  assert(!io.readPage(133,buf));
  const uint8_t illegal[4]={1,2,3,4};
  assert(!io.writePage(2,illegal) && !io.writePage(128,illegal));
  assert(reader.writes==0);
  const uint8_t uid[7]={0x04,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};
  const char *const url="https://vault.example.com/t/MV-A3F92C10/";
  assert(MakerVaultNtag215::writeNdef(io,uid,"04AABBCCDDEEFF","MV-A3F92C10",url,true,true)
         ==MakerVaultNtag215::RESULT_VERIFIED);
  assert(reader.writes>0);
  const uint8_t originalCC[4]={0xE1,0x10,0x3E,0};
  for(int i=0;i<4;++i) assert(reader.memory[3][i] == originalCC[i]);
  puts("MakerVault RC522 page adapter mock test: OK");
}
