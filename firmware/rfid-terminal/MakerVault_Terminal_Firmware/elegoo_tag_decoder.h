#pragma once
// Experimental ELEGOO NTAG213 payload parser. Pure, READ-ONLY memory decoder.
// The manufacturer's public guide is internally inconsistent about EPC-256
// byte counts; this decoder accepts only a narrow explicitly verified field
// sequence and never treats arbitrary NTAG215 MakerTags as Elegoo material.
#include <stddef.h>
#include <stdint.h>
#include <string.h>
namespace ElegooTag {
struct Material {
  char material[5];
  char subtype[5];
  char color[7];
  uint16_t diameterHundredths;
  uint16_t weightGrams;
};
inline char hex(uint8_t x){return char(x<10?'0'+x:'A'+x-10);}
inline bool asciiField(const uint8_t *src,char dest[5]){
  // The documented 4-character field examples use either ASCII or a nonstandard
  // encoding; only unambiguous printable ASCII is currently accepted.
  size_t n=0;
  while(n<4 && src[n] && src[n]!=' ') {
    if(src[n]<32 || src[n]>126)return false;
    dest[n]=char(src[n]);++n;
  }
  if(!n)return false;
  dest[n]=0;return true;
}
// Normalised payload fixture: 0x36 + EEEEEEEE + 2-byte product code +
// 4-byte main material + 4-byte subtype + 3-byte RGB + 2-byte diameter +
// 2-byte weight. Caller must independently establish tag family/placement.
// This is NOT a generic EPC parser and doesn't imply tag readability.
inline bool parseDocumentedFields(const uint8_t *data,size_t len,Material &out){
  if(!data || len<22 || data[0]!=0x36)return false;
  for(size_t i=1;i<=4;i++)if(data[i]!=0xEE)return false;
  Material candidate={};
  if(!asciiField(data+7,candidate.material) ||
     !asciiField(data+11,candidate.subtype)) return false;
  for(size_t i=0;i<3;i++){
    candidate.color[i*2]=hex(data[15+i]>>4);
    candidate.color[i*2+1]=hex(data[15+i]&15);
  }
  candidate.color[6]=0;
  candidate.diameterHundredths=uint16_t(data[18])<<8 | data[19];
  candidate.weightGrams=uint16_t(data[20])<<8 | data[21];
  if(candidate.diameterHundredths<50 || candidate.diameterHundredths>500 ||
     candidate.weightGrams<50 || candidate.weightGrams>10000)return false;
  out=candidate;return true;
}
}
