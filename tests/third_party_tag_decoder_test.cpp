#include <cassert>
#include <cstdint>
#include <cstring>
#include "../firmware/rfid-terminal/MakerVault_Terminal_Firmware/bambu_tag_decoder.h"
#include "../firmware/rfid-terminal/MakerVault_Terminal_Firmware/elegoo_tag_decoder.h"

int main() {
  uint8_t b1[16]={}, b2[16]={}, b4[16]={}, b5[16]={};
  std::memcpy(b1,"A50-K0",6);
  std::memcpy(b1+8,"GFA50",5);
  std::memcpy(b2,"PLA",3);
  std::memcpy(b4,"PLA Basic",9);
  b5[0]=0x29;b5[1]=0xE0;b5[2]=0x00;b5[3]=0xff;
  b5[4]=0xe8;b5[5]=0x03;
  BambuTag::Material bambu={};
  assert(BambuTag::parse(b1,b2,b4,b5,bambu));
  assert(std::strcmp(bambu.materialId,"GFA50")==0);
  assert(std::strcmp(bambu.detail,"PLA Basic")==0);
  assert(std::strcmp(bambu.color,"29E000")==0);
  assert(bambu.weightGrams==1000);
  b1[8]='X';assert(!BambuTag::parse(b1,b2,b4,b5,bambu));b1[8]='G';
  b5[4]=0;b5[5]=0;assert(!BambuTag::parse(b1,b2,b4,b5,bambu));
  assert(!BambuTag::parse(nullptr,b2,b4,b5,bambu));
  // Synthetic field-sequence fixtures from published plain ASCII example.
  // These test parsing only, not actual NTAG page placement or readability.
  uint8_t e[22]={0x36,0xee,0xee,0xee,0xee,0,1,
    'P','L','A',' ', 'C','F',' ',' ', 0xff,0x37,0x00,
    0x00,0xaf,0x03,0xe8};
  ElegooTag::Material elegoo={};
  assert(ElegooTag::parseDocumentedFields(e,sizeof(e),elegoo));
  assert(std::strcmp(elegoo.material,"PLA")==0);
  assert(std::strcmp(elegoo.subtype,"CF")==0);
  assert(std::strcmp(elegoo.color,"FF3700")==0);
  assert(elegoo.diameterHundredths==175 && elegoo.weightGrams==1000);
  assert(!ElegooTag::parseDocumentedFields(e,21,elegoo));
  e[1]=0;assert(!ElegooTag::parseDocumentedFields(e,22,elegoo));e[1]=0xee;
  e[7]=0x80;assert(!ElegooTag::parseDocumentedFields(e,22,elegoo));e[7]='P';
  e[20]=0;e[21]=0;assert(!ElegooTag::parseDocumentedFields(e,22,elegoo));
  assert(!ElegooTag::parseDocumentedFields(nullptr,22,elegoo));
}
