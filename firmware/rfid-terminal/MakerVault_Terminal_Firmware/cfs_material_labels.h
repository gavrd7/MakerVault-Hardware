#pragma once
// Local colour rendering only. Material names require online MakerVault lookup.
#include <string.h>
#include <stdint.h>
namespace CfsTag {
// Names are provided by MakerVault at runtime; no bundled material catalogue.
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
