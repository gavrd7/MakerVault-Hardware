#pragma once
// Read-only NTAG215 inspection for the MFRC522 reader.
// No MIFARE_Ultralight_Write, PCD_TransceiveData write opcode, or lock updates.
// Must be called only while a selected PICC is present. A successful read is
// NOT proof of a safe write; the caller must separately authorize and confirm.
#include <MFRC522.h>
#include "ntag215_inspection.h"

namespace MakerVaultNtagReader {
struct Snapshot {
  MakerVaultNtag215::MemoryInfo memory = {false, false, 0};
  bool complete = false;
  uint8_t cc[4] = {};
  uint8_t staticLock[2] = {};
  uint8_t dynamicLock[3] = {};
  uint8_t auth0 = 0;
  uint8_t access = 0;
};

// MFRC522 MIFARE_Read retrieves four pages (16 bytes) per command.
// Do not change this to writing or authentication without a separate review.
inline bool readFour(MFRC522 &reader, uint8_t page, uint8_t out[16]) {
  byte size = 18; // Include space for the library's CRC bytes.
  byte response[18] = {};
  MFRC522::StatusCode status = reader.MIFARE_Read(page, response, &size);
  if (status != MFRC522::STATUS_OK || size != 18) return false;
  for (unsigned i = 0; i < 16; ++i) out[i] = response[i];
  return true;
}

inline Snapshot inspectSelected(MFRC522 &reader) {
  Snapshot result;
  if (reader.uid.size != 7 ||
      reader.PICC_GetType(reader.uid.sak) != MFRC522::PICC_TYPE_MIFARE_UL)
    return result;

  uint8_t pages2to5[16] = {};
  uint8_t pages130to133[16] = {};
  uint8_t pages131to134[16] = {};
  if (!readFour(reader, 2, pages2to5) ||
      !readFour(reader, 130, pages130to133) ||
      !readFour(reader, 131, pages131to134))
    return result;
  result.staticLock[0] = pages2to5[2];
  result.staticLock[1] = pages2to5[3];
  for (unsigned i = 0; i < 4; ++i) result.cc[i] = pages2to5[4 + i];
  for (unsigned i = 0; i < 3; ++i)
    result.dynamicLock[i] = pages130to133[i];

  // NTAG215 configuration pages 131-134: AUTH0 on 131 byte 3,
  // ACCESS on 132 byte 0. Any configured protection fails closed.
  result.auth0 = pages131to134[3];
  result.access = pages131to134[4];
  result.memory = MakerVaultNtag215::inspect(
      reader.uid.uidByte, reader.uid.size, result.cc,
      result.staticLock, result.dynamicLock);
  if (result.auth0 != 0xFF || result.access != 0x00)
    result.memory.writable = false;
  result.complete = result.memory.supported;
  return result;
}
} // namespace MakerVaultNtagReader
