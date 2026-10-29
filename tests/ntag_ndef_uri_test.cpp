// Portable host test for the firmware NDEF URI codec.
// Compile: c++ -std=c++11 -Wall -Wextra -Werror tests/ntag_ndef_uri_test.cpp -o /tmp/ndef-test && /tmp/ndef-test
#include "../firmware/rfid-terminal/MakerVault_Terminal_Firmware/ntag_ndef_uri.h"
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
  puts("MakerVault NTAG215 NDEF codec: OK");
}
