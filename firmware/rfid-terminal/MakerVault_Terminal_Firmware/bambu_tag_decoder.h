#pragma once
// Experimental Bambu Lab decoder for already authenticated MIFARE Classic blocks.
// Pure parser: no authentication, RFID I/O, writes, cloud lookup or spool binding.
#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace BambuTag {
struct Material {
  char variantId[9];
  char materialId[9];
  char material[17];
  char detail[17];
  char color[7];
  uint16_t weightGrams;
};
inline bool field(const uint8_t *p,size_t n,char *out,size_t capacity) {
  if (!p || n + 1 > capacity) return false;
  size_t end=n;
  while(end && (p[end-1]==0 || p[end-1]==' ')) --end;
  if (!end) return false;
  for(size_t i=0;i<end;i++) if(p[i]<32 || p[i]>126) return false;
  memcpy(out,p,end);out[end]=0;return true;
}
inline char hex(uint8_t v) { return v<10 ? char('0'+v) : char('A'+v-10); }
// Bambu research format: blocks 1,2,4,5 are 16-byte plaintext blocks.
// Call only after the device has successfully authenticated and read the blocks.
inline bool parse(const uint8_t *block1,const uint8_t *block2,
                  const uint8_t *block4,const uint8_t *block5,Material &out) {
  if (!block1 || !block2 || !block4 || !block5) return false;
  Material candidate={};
  if(!field(block1,8,candidate.variantId,sizeof(candidate.variantId)) ||
     !field(block1+8,8,candidate.materialId,sizeof(candidate.materialId)) ||
     !field(block2,16,candidate.material,sizeof(candidate.material)) ||
     !field(block4,16,candidate.detail,sizeof(candidate.detail))) return false;
  // A documented family marker avoids misclassifying arbitrary Classic tags.
  if(candidate.materialId[0]!='G' || candidate.materialId[1]!='F')return false;
  for(size_t i=0;i<3;i++) {
    candidate.color[i*2]=hex(block5[i]>>4);
    candidate.color[i*2+1]=hex(block5[i]&15);
  }
  candidate.color[6]=0;
  // RGBA alpha can describe a translucent colour; retain RGB only.
  candidate.weightGrams=uint16_t(block5[4]) | (uint16_t(block5[5])<<8);
  if(candidate.weightGrams==0 || candidate.weightGrams>10000)return false;
  out=candidate;return true;
}
}
