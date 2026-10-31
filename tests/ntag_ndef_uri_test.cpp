// Portable host test for the firmware NDEF URI codec.
// Compile: c++ -std=c++11 -Wall -Wextra -Werror tests/ntag_ndef_uri_test.cpp -o /tmp/ndef-test && /tmp/ndef-test
#include "../firmware/rfid-terminal/MakerVault_Terminal_Firmware/ntag_ndef_uri.h"
#include "../firmware/rfid-terminal/MakerVault_Terminal_Firmware/ntag215_inspection.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main() {
  const char *cases[] = {
    "http://192.168.1.125:8000/t/MV-A3F92C10/",
    "https://vault.example.com:8443/t/MV-1234ABCD/",
    "http://maker.local:8000/t/MV-12345678/",
  };
  for (const char *url : cases) {
    uint8_t encoded[MakerVaultNdef::kMaxEncodedLength] = {0};
    size_t length = 0;
    assert(MakerVaultNdef::encodeUri(url, encoded, sizeof(encoded), length));
    assert(length == strlen(url) + 8);
    assert(encoded[0] == 0x03 && encoded[2] == 0xD1 &&
           encoded[5] == 'U' && encoded[length - 1] == 0xFE);
    char decoded[MakerVaultNdef::kMaxUriLength + 1] = {0};
    assert(MakerVaultNdef::decodeUri(encoded, length, decoded, sizeof(decoded)));
    assert(strcmp(decoded, url) == 0);
    encoded[5] = 'X';
    assert(!MakerVaultNdef::decodeUri(encoded, length, decoded, sizeof(decoded)));
  }
  uint8_t encoded[MakerVaultNdef::kMaxEncodedLength] = {0};
  size_t length = 0;
  assert(!MakerVaultNdef::encodeUri("javascript:alert(1)", encoded, sizeof(encoded), length));
  assert(!MakerVaultNdef::encodeUri("https://bad host/", encoded, sizeof(encoded), length));
  assert(!MakerVaultNdef::encodeUri("http://host/", encoded, 4, length));
  assert(!MakerVaultNdef::decodeUri(encoded, 7, nullptr, 0));
  const uint8_t uid[7] = {0x04,0x11,0x22,0x33,0x44,0x55,0x66};
  const uint8_t cc[4] = {0xE1,0x10,0x3E,0x00};
  const uint8_t clear[3] = {0,0,0};
  const uint8_t lock[2] = {0,0};
  auto info = MakerVaultNtag215::inspect(uid, 7, cc, lock, clear);
  assert(info.supported && info.writable && info.advertisedUserBytes == 496);
  assert(!MakerVaultNtag215::inspect(uid, 4, cc, lock, clear).supported);
  const uint8_t otherCc[4] = {0xE1,0x10,0x12,0};
  assert(!MakerVaultNtag215::inspect(uid,7,otherCc,lock,clear).supported);
  const uint8_t locked[2] = {0xFF,0};
  info = MakerVaultNtag215::inspect(uid, 7, cc, locked, clear);
  assert(info.supported && !info.writable);
  const uint8_t dynamicLocked[3] = {0,1,0};
  assert(!MakerVaultNtag215::inspect(uid,7,cc,lock,dynamicLocked).writable);
  const uint8_t readOnlyCc[4] = {0xE1,0x10,0x3E,0x0F};
  assert(!MakerVaultNtag215::inspect(uid,7,readOnlyCc,lock,clear).writable);
  // Boundaries: max-length URL fits, one extra byte fails safely.
  char longUrl[MakerVaultNdef::kMaxUriLength + 2] = {0};
  memcpy(longUrl, "https://", 8);
  memset(longUrl + 8, 'a', MakerVaultNdef::kMaxUriLength - 8);
  longUrl[MakerVaultNdef::kMaxUriLength] = '\0';
  assert(MakerVaultNdef::encodeUri(longUrl, encoded, sizeof(encoded), length));
  assert(length == MakerVaultNdef::kMaxEncodedLength);
  char decodedLong[MakerVaultNdef::kMaxUriLength + 1] = {0};
  assert(MakerVaultNdef::decodeUri(encoded, length, decodedLong, sizeof(decodedLong)));
  assert(strcmp(decodedLong, longUrl) == 0);
  longUrl[MakerVaultNdef::kMaxUriLength] = 'a';
  longUrl[MakerVaultNdef::kMaxUriLength + 1] = '\0';
  assert(!MakerVaultNdef::encodeUri(longUrl, encoded, sizeof(encoded), length));
  assert(length == 0);
  assert(!MakerVaultNdef::encodeUri(nullptr, encoded, sizeof(encoded), length));
  assert(!MakerVaultNdef::encodeUri("https://example.com/", nullptr, sizeof(encoded), length));

  // Truncation and corruption must not produce a believable verification URL.
  assert(MakerVaultNdef::encodeUri("https://maker.example/t/MV-12345678/",
                                  encoded, sizeof(encoded), length));
  char verify[MakerVaultNdef::kMaxUriLength + 1] = {0};
  assert(!MakerVaultNdef::decodeUri(encoded, length - 1, verify, sizeof(verify)));
  assert(!MakerVaultNdef::decodeUri(encoded, length, verify, 4));
  const uint8_t originalLength = encoded[1];
  encoded[1] = 0xFF;
  assert(!MakerVaultNdef::decodeUri(encoded, length, verify, sizeof(verify)));
  encoded[1] = originalLength;
  encoded[length - 1] = 0x00;
  assert(!MakerVaultNdef::decodeUri(encoded, length, verify, sizeof(verify)));
  encoded[length - 1] = 0xFE;
  encoded[4] = 0xFF; // Declared payload length no longer matches NDEF TLV.
  assert(!MakerVaultNdef::decodeUri(encoded, length, verify, sizeof(verify)));

  // Fail closed on malformed CC and lock bytes.
  const uint8_t malformedCc[4] = {0xE1,0x10,0x3E,0x01};
  assert(!MakerVaultNtag215::inspect(uid, 7, malformedCc, lock, clear).writable);
  const uint8_t staticLocked[2] = {0,0x01};
  assert(!MakerVaultNtag215::inspect(uid, 7, cc, staticLocked, clear).writable);
  assert(!MakerVaultNtag215::inspect(nullptr, 7, cc, lock, clear).supported);
  puts("MakerVault NTAG215 NDEF codec and memory inspection: OK");
}
