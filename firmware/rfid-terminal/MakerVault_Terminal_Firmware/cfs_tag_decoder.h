#pragma once
// Creality CFS MIFARE Classic sector payload parser. READ-ONLY.
// Input is already authenticated/decrypted 48-byte sector-1 data. This
// module deliberately performs no RFID operations, writes or printer sync.
#include <stddef.h>
#include <stdint.h>

namespace CfsTag {
struct Material {
  char materialId[6]; // five characters + NUL
  char color[7];      // six ASCII hex characters + NUL
  char weightCode[5]; // four characters + NUL
};

inline bool asciiVisible(uint8_t c) { return c >= 0x20 && c <= 0x7e; }
inline bool asciiHex(uint8_t c) {
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

// Offsets taken from CFS-RFID-ESP32 performTagRead. This deliberately does
// not infer a tag's authenticity or use fields as database update commands.
inline bool parseSector1(const uint8_t *data, size_t len, Material &out) {
  if (!data || len != 48) return false;
  const char prefix[] = {'A','B','1','2','4'};
  for (size_t i=0; i<5; ++i) if (data[i] != uint8_t(prefix[i])) return false;
  for (size_t i=12; i<17; ++i) if (!asciiVisible(data[i])) return false;
  for (size_t i=18; i<24; ++i) if (!asciiHex(data[i])) return false;
  for (size_t i=24; i<28; ++i) if (!asciiVisible(data[i])) return false;
  for (size_t i=0; i<5; ++i) out.materialId[i] = char(data[12+i]);
  out.materialId[5] = '\0';
  for (size_t i=0; i<6; ++i) out.color[i] = char(data[18+i]);
  out.color[6] = '\0';
  for (size_t i=0; i<4; ++i) out.weightCode[i] = char(data[24+i]);
  out.weightCode[4] = '\0';
  return true;
}
inline const char *weightLabel(const char *code) {
  if (!code) return "Unknown";
  if (code[0]=='0' && code[1]=='3' && code[2]=='3' && code[3]=='0' && code[4]=='\0') return "1 KG";
  if (code[0]=='0' && code[1]=='2' && code[2]=='0' && code[3]=='0' && code[4]=='\0') return "750 G";
  if (code[0]=='0' && code[1]=='0' && code[2]=='8' && code[3]=='2' && code[4]=='\0') return "250 G";
  return "Unknown";
}
}
