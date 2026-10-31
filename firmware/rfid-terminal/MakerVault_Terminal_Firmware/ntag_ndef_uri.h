#pragma once
// MakerVault NTAG215 NDEF URI codec; physical memory I/O intentionally separate.
// Supports one uncompressed NFC Forum URI record. No lock/OTP writes.
#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace MakerVaultNdef {
static constexpr size_t kMaxUriLength = 240;
static constexpr size_t kMaxEncodedLength = kMaxUriLength + 8;

inline bool validUri(const char *uri, size_t length) {
  if (!uri || !length || length > kMaxUriLength) return false;
  const bool https = length >= 8 && memcmp(uri, "https://", 8) == 0;
  const bool http = length >= 7 && memcmp(uri, "http://", 7) == 0;
  if (!https && !http) return false;
  for (size_t i = 0; i < length; ++i)
    if (static_cast<unsigned char>(uri[i]) <= 0x20 ||
        static_cast<unsigned char>(uri[i]) >= 0x7F) return false;
  return true;
}

// Returns a Type 2 user-memory TLV sequence (03, length, NDEF, FE).
// Caller MUST check the physical tag CC, lock bits, writable range and UID.
inline bool encodeUri(const char *uri, uint8_t *out, size_t capacity, size_t &written) {
  written = 0;
  if (!uri || !out) return false;
  const size_t uriLength = strlen(uri);
  if (!validUri(uri, uriLength) || capacity < uriLength + 8) return false;
  out[0] = 0x03;                       // NDEF TLV
  out[1] = static_cast<uint8_t>(uriLength + 5);
  out[2] = 0xD1;                       // MB|ME|SR|Well-known
  out[3] = 0x01;                       // Type length
  out[4] = static_cast<uint8_t>(uriLength + 1);
  out[5] = 0x55;                       // URI type 'U'
  out[6] = 0x00;                       // No prefix compression
  memcpy(out + 7, uri, uriLength);
  out[uriLength + 7] = 0xFE;           // Terminator TLV
  written = uriLength + 8;
  return true;
}

// Only accepts the single uncompressed URI record we emit; rejects unknown
// TLV or NDEF formats instead of guessing what another writer meant.
inline bool decodeUri(const uint8_t *bytes, size_t length, char *out, size_t capacity) {
  if (!bytes || !out || length < 8) return false;
  size_t i = 0;
  while (i < length && bytes[i] == 0x00) ++i;
  if (i + 8 > length || bytes[i] != 0x03) return false;
  const size_t ndefLength = bytes[i + 1];
  if (ndefLength < 6 || ndefLength >= 0xFF ||
      i + 2 + ndefLength >= length) return false;
  const uint8_t *n = bytes + i + 2;
  if (n[0] != 0xD1 || n[1] != 1 || n[3] != 0x55 || n[4] != 0x00)
    return false;
  const size_t payloadLength = n[2];
  if (ndefLength != payloadLength + 4 || payloadLength < 2) return false;
  const size_t uriLength = payloadLength - 1;
  if (capacity <= uriLength || bytes[i + 2 + ndefLength] != 0xFE)
    return false;
  memcpy(out, n + 5, uriLength);
  out[uriLength] = '\0';
  return validUri(out, uriLength);
}
} // namespace MakerVaultNdef
