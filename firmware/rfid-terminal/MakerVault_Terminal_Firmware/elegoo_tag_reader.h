#pragma once
// Experimental read-only NTAG213 field acquisition for published ELEGOO layout.
// No PWD_AUTH, no writes, and never called during MakerTag/NDEF workflows.
#include <MFRC522.h>
#include <string.h>
#include "elegoo_tag_decoder.h"
namespace ElegooTag {
inline bool readDocumentedTag(MFRC522 &reader, Material &out) {
  if (reader.PICC_GetType(reader.uid.sak) != MFRC522::PICC_TYPE_MIFARE_UL)
    return false;
  uint8_t bytes[24]={};
  // Read 4 pages per MIFARE Ultralight command. These overlapping calls
  // acquire pages 4..11 then 8..11, and 12..15; 24 bytes total.
  for (uint8_t page=4;page<=8;page+=4) {
    uint8_t response[18]={};
    byte length=sizeof(response);
    if (reader.MIFARE_Read(page,response,&length)!=MFRC522::STATUS_OK ||
        length < 16) return false;
    memcpy(bytes+(page-4)*4,response,16);
  }
  return parseDocumentedFields(bytes,sizeof(bytes),out);
}
}
