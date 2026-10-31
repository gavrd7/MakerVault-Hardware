#pragma once
// MFRC522 adapter for isolated NTAG215 page operations.
// It MUST only be used after a genuine NTAG215 selection, trusted server
// operation, complete CC/lock/config inspection and physical confirmation.
// Not included or called by the published scanner.
#ifndef MAKERVAULT_MFRC522_TEST
#include <MFRC522.h>
#endif
#include "ntag215_page_transaction.h"

class MakerVaultRc522PageIO : public MakerVaultNtag215::PageIO {
 public:
  explicit MakerVaultRc522PageIO(MFRC522 &reader) : reader_(reader) {}
  bool readPage(uint8_t page, uint8_t out[4]) override {
    if (!out || page > 132) return false;
    uint8_t buffer[18] = {};
    uint8_t count = sizeof(buffer);
    // READ returns four consecutive pages. Start at 130 when requesting
    // late NTAG215 configuration pages to avoid crossing the tag end.
    const uint8_t start = page >= 130 ? 130 : page;
    if (reader_.MIFARE_Read(start, buffer, &count) != MFRC522::STATUS_OK ||
        count < 16) return false;
    memcpy(out, buffer + (page-start)*4, 4);
    return true;
  }
  bool writePage(uint8_t page, const uint8_t bytes[4]) override {
    // Never permit writes to manufacturer, UID, CC, OTP, locks, configuration,
    // password, PACK or pages outside the conservative CC-advertised region.
    if (!bytes || page < 4 || page > 127) return false;
    uint8_t copy[4];
    memcpy(copy, bytes, sizeof(copy));
    return reader_.MIFARE_Ultralight_Write(page,copy,4) == MFRC522::STATUS_OK;
  }
 private:
  MFRC522 &reader_;
};
