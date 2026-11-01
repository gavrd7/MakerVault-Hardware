#pragma once
// Presentation-only CFS material-name mapping. No tag, printer or DB writes.
// Known entries mirror the read-only CFS-RFID display catalogue. Unknown IDs
// retain their identifier rather than inventing a manufacturer or product.
#include <string.h>
namespace CfsTag {
struct MaterialLabel { const char *id; const char *manufacturer; const char *name; };
static constexpr MaterialLabel kMaterialLabels[] = {
 {"01001","Creality","Hyper PLA"},
 {"02001","Creality","Hyper PLA-CF"},
 {"06002","Creality","Hyper PETG"},
 {"03001","Creality","Hyper ABS"},
 {"04001","Creality","CR-PLA"},
 {"05001","Creality","CR-Silk"},
 {"06001","Creality","CR-PETG"},
 {"07001","Creality","CR-ABS"},
 {"08001","Creality","Ender-PLA"},
 {"09001","Creality","EN-PLA+"},
 {"10001","Creality","HP-TPU"},
 {"11001","Creality","CR-Nylon"},
 {"13001","Creality","CR-PLA Carbon"},
 {"14001","Creality","CR-PLA Matte"},
 {"15001","Creality","CR-PLA Fluo"},
 {"16001","Creality","CR-TPU"},
 {"17001","Creality","CR-Wood"},
 {"18001","Creality","HP Ultra PLA"},
 {"19001","Creality","HP-ASA"},
 {"00001","Generic","Generic PLA"},
 {"00002","Generic","Generic PLA-Silk"},
 {"00003","Generic","Generic PETG"},
 {"00004","Generic","Generic ABS"},
 {"00005","Generic","Generic TPU"},
 {"00006","Generic","Generic PLA-CF"},
 {"00007","Generic","Generic ASA"},
 {"00008","Generic","Generic PA"},
 {"00009","Generic","Generic PA-CF"},
 {"00010","Generic","Generic BVOH"},
 {"00011","Generic","Generic PVA"},
 {"00012","Generic","Generic HIPS"},
 {"00013","Generic","Generic PET-CF"},
 {"00014","Generic","Generic PETG-CF"},
 {"00015","Generic","Generic PA6-CF"},
 {"00016","Generic","Generic PAHT-CF"},
 {"00017","Generic","Generic PPS"},
 {"00018","Generic","Generic PPS-CF"},
 {"00019","Generic","Generic PP"},
 {"00020","Generic","Generic PET"},
 {"00021","Generic","Generic PC"},
 // Confirmed against the same physical tag's CFS-RFID display (2026-11-01).
 {"10437","eSUN","PLA Basic"},
};
inline const MaterialLabel *findMaterialLabel(const char *id) {
 if(!id) return nullptr;
 for(const auto &row : kMaterialLabels)
   if(strcmp(row.id,id)==0) return &row;
 return nullptr;
}
inline uint16_t hexRgb565(const char *hex) {
 if(!hex) return 0;
 auto digit=[](char c)->uint8_t {
  if(c>='0'&&c<='9')return uint8_t(c-'0');
  if(c>='A'&&c<='F')return uint8_t(c-'A'+10);
  if(c>='a'&&c<='f')return uint8_t(c-'a'+10);
  return 0;
 };
 uint8_t r=uint8_t((digit(hex[0])<<4)|digit(hex[1]));
 uint8_t g=uint8_t((digit(hex[2])<<4)|digit(hex[3]));
 uint8_t b=uint8_t((digit(hex[4])<<4)|digit(hex[5]));
 return uint16_t(((r&0xF8)<<8)|((g&0xFC)<<3)|(b>>3));
}
}
