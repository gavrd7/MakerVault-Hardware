#pragma once
// Pure pre-write policy: no NFC I/O, no hardware commands, no global state.
// The caller must separately authenticate a live device operation and obtain
// user confirmation. A positive result is NOT permission to bypass RC522
// card identification, CC/lock checks or post-write read-back verification.
#include "ntag_ndef_uri.h"
#include "ntag215_inspection.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace MakerVaultNtag215 {

enum WriteDecision {
  WRITE_READY,
  WRITE_REJECT_CONFIRMATION,
  WRITE_REJECT_UNSUPPORTED,
  WRITE_REJECT_LOCKED,
  WRITE_REJECT_UID,
  WRITE_REJECT_URL,
  WRITE_REJECT_FOREIGN_CONTENT
};

inline int hexNibble(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return -1;
}

inline bool matchesUid(const uint8_t *uid, size_t length, const char *expectedUid) {
  if (!uid || !expectedUid || length != 7 || strlen(expectedUid) != 14) return false;
  for (size_t i = 0; i < 7; ++i) {
    const int h = hexNibble(expectedUid[i * 2]);
    const int l = hexNibble(expectedUid[i * 2 + 1]);
    if (h < 0 || l < 0 || uid[i] != static_cast<uint8_t>((h << 4) | l)) return false;
  }
  return true;
}

inline bool hasIdentitySuffix(const char *url, const char *makerTagId) {
  if (!url || !makerTagId || strlen(makerTagId) != 11 ||
      strncmp(makerTagId, "MV-", 3) != 0) return false;
  for (size_t i = 3; i < 11; ++i)
    if (hexNibble(makerTagId[i]) < 0) return false;
  const size_t length = strlen(url);
  const size_t suffixLength = 3 + 11 + 1;  // /t/ + MV-XXXXXXXX + /
  if (length < suffixLength || !MakerVaultNdef::validUri(url, length)) return false;
  const char *suffix = url + length - suffixLength;
  return strncmp(suffix, "/t/", 3) == 0 &&
         strncmp(suffix + 3, makerTagId, 11) == 0 && suffix[14] == '/';
}

// Blank memory is accepted only when the captured user memory contains no
// meaningful bytes or an empty NDEF TLV. A foreign or unparseable NDEF is
// rejected, including legacy links. Never silently overwrite it.
inline bool blankNdef(const uint8_t *memory, size_t count) {
  if (!memory || count == 0) return false;
  size_t i = 0;
  while (i < count && memory[i] == 0x00) ++i;
  if (i == count) return true;
  if (i + 2 < count && memory[i] == 0x03 && memory[i + 1] == 0x00 &&
      memory[i + 2] == 0xFE) {
    // Reject hidden content after an empty-record terminator. The caller
    // must never interpret a partially inspected/dirty tag as blank.
    for (size_t j = i + 3; j < count; ++j)
      if (memory[j] != 0x00) return false;
    return true;
  }
  return false;
}

inline WriteDecision assess(const MemoryInfo &info, const uint8_t *uid,
                            size_t uidLength, const char *expectedUid,
                            const char *makerTagId, const char *proposedUrl,
                            const uint8_t *existing, size_t existingLength,
                            bool physicalConfirmed, bool serverAuthorized) {
  if (!physicalConfirmed || !serverAuthorized) return WRITE_REJECT_CONFIRMATION;
  if (!info.supported || info.advertisedUserBytes != 496) return WRITE_REJECT_UNSUPPORTED;
  if (!info.writable) return WRITE_REJECT_LOCKED;
  if (!matchesUid(uid, uidLength, expectedUid)) return WRITE_REJECT_UID;
  if (!hasIdentitySuffix(proposedUrl, makerTagId)) return WRITE_REJECT_URL;
  if (blankNdef(existing, existingLength)) return WRITE_READY;
  char previous[MakerVaultNdef::kMaxUriLength + 1] = {0};
  if (!MakerVaultNdef::decodeUri(existing, existingLength, previous, sizeof(previous)))
    return WRITE_REJECT_FOREIGN_CONTENT;
  if (!hasIdentitySuffix(previous, makerTagId)) return WRITE_REJECT_FOREIGN_CONTENT;
  return WRITE_READY;
}
// Independently verify what was physically read back after a write.
// Caller must re-select the tag and provide fresh UID and page reads; do not
// reuse the pre-write snapshot as proof of a completed write.
inline bool verifyReadback(const uint8_t *uid, size_t uidLength,
                           const char *expectedUid, const char *expectedUrl,
                           const uint8_t *readback, size_t readbackLength) {
  if (!matchesUid(uid, uidLength, expectedUid) || !expectedUrl ||
      !MakerVaultNdef::validUri(expectedUrl, strlen(expectedUrl))) return false;
  char actual[MakerVaultNdef::kMaxUriLength + 1] = {0};
  return MakerVaultNdef::decodeUri(readback, readbackLength,
                                  actual, sizeof(actual)) &&
         strcmp(actual, expectedUrl) == 0;
}

} // namespace MakerVaultNtag215
