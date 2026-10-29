#pragma once
// Isolated NTAG215 user-memory transaction. NOT wired to the live RC522 sketch.
// Page I/O must come from a separately tested driver; no configuration,
// manufacturer, CC, lock, OTP or password pages are ever written.
#include "ntag215_write_policy.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

namespace MakerVaultNtag215 {
static const size_t kUserBytes = 496;        // CC 0x3E * 8
static const uint8_t kFirstUserPage = 4;     // data begins at page 4
static const uint8_t kDynamicLockPage = 130; // outside writable NDEF area

class PageIO {
 public:
  virtual bool readPage(uint8_t page, uint8_t result[4]) = 0;
  virtual bool writePage(uint8_t page, const uint8_t bytes[4]) = 0;
  virtual ~PageIO() {}
};

enum WriteResult {
  RESULT_VERIFIED,
  RESULT_READ_FAILED,
  RESULT_UNSAFE,
  RESULT_INVALID_URL,
  RESULT_WRITE_FAILED,
  RESULT_READBACK_MISMATCH
};

inline bool readNdefArea(PageIO &io, uint8_t buffer[kUserBytes]) {
  for (size_t i = 0; i < kUserBytes / 4; ++i)
    if (!io.readPage(static_cast<uint8_t>(kFirstUserPage + i), buffer + i*4))
      return false;
  return true;
}

inline bool inspectMemory(PageIO &io, const uint8_t uid[7], MemoryInfo &info) {
  uint8_t lockPage[4] = {}, cc[4] = {}, dynamic[4] = {};
  uint8_t cfg0[4] = {}, cfg1[4] = {};
  if (!io.readPage(2, lockPage) || !io.readPage(3, cc) ||
      !io.readPage(kDynamicLockPage, dynamic) ||
      !io.readPage(131, cfg0) || !io.readPage(132, cfg1)) return false;
  uint8_t staticLocks[2] = {lockPage[2],lockPage[3]};
  info = inspect(uid,7,cc,staticLocks,dynamic);
  // NTAG215 CFG0 page 0x83 byte 3: AUTH0 must be FF (password off).
  // CFG1 page 0x84 byte 0: ACCESS must be 00 (default/unprotected).
  // Nondefault mirror configuration is deliberately unsupported.
  if (cfg0[3] != 0xFF || cfg1[0] != 0x00 || cfg0[0] != 0x00)
    info.writable = false;
#ifndef MAKERVAULT_MFRC522_TEST
  // Diagnostic only. Do not expose authentication secrets or change locks.
  // Helps distinguish genuine lock bits from a non-default configuration.
  Serial.printf("[NDEF] CC=%02X %02X %02X %02X static=%02X %02X dynamic=%02X %02X %02X cfg0[0]=%02X AUTH0=%02X ACCESS=%02X supported=%d writable=%d\\n",
    cc[0],cc[1],cc[2],cc[3],staticLocks[0],staticLocks[1],
    dynamic[0],dynamic[1],dynamic[2],cfg0[0],cfg0[3],cfg1[0],
    info.supported,info.writable);
#endif
  return true;
}

// Caller MUST have rechecked server request and pressed the physical Confirm
// button. Power loss between pages leaves a zero-length TLV, not a success.
// Verifying after reselecting the physical UID remains the caller's job.
inline WriteResult writeNdef(PageIO &io, const uint8_t uid[7],
                             const char *expectedUid, const char *tagId,
                             const char *url, bool confirmed, bool authorized) {
  uint8_t encoded[MakerVaultNdef::kMaxEncodedLength] = {};
  size_t written=0;
  if (!MakerVaultNdef::encodeUri(url,encoded,sizeof(encoded),written))
    return RESULT_INVALID_URL;
  MemoryInfo info = {false,false,0};
  if (!inspectMemory(io,uid,info)) return RESULT_READ_FAILED;
  uint8_t oldData[kUserBytes] = {};
  if (!readNdefArea(io,oldData)) return RESULT_READ_FAILED;
  if (assess(info,uid,7,expectedUid,tagId,url,oldData,sizeof(oldData),
             confirmed,authorized) != WRITE_READY) return RESULT_UNSAFE;

  // The length is cleared first and written last, after all payload pages.
  uint8_t first[4] = {encoded[0],0x00,encoded[2],encoded[3]};
  if (!io.writePage(kFirstUserPage,first)) return RESULT_WRITE_FAILED;
  size_t pages=(written+3)/4;
  for(size_t page=1;page<pages;++page) {
    uint8_t bytes[4]={};
    for(size_t b=0;b<4;++b)
      if(page*4+b<written) bytes[b]=encoded[page*4+b];
    if (!io.writePage(static_cast<uint8_t>(kFirstUserPage+page),bytes))
      return RESULT_WRITE_FAILED;
  }
  memcpy(first,encoded,4);
  if (!io.writePage(kFirstUserPage,first)) return RESULT_WRITE_FAILED;
  uint8_t readback[kUserBytes] = {};
  if (!readNdefArea(io,readback) ||
      memcmp(readback,encoded,written)!=0 ||
      !verifyReadback(uid,7,expectedUid,url,readback,sizeof(readback)))
    return RESULT_READBACK_MISMATCH;
  return RESULT_VERIFIED;
}
} // namespace MakerVaultNtag215
