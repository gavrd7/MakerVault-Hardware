#pragma once
// READ-ONLY Creality CFS MIFARE Classic material-sector reader.
// No MIFARE_Write, NDEF write, printer connection or database mutation.
// The known CFS format and fixed AES keys are from gavrd7/CFS-RFID-ESP32;
// tag content is user-programmable and must not be trusted as authentication.
#include <MFRC522.h>
#include <mbedtls/aes.h>
#include <string.h>
#include "cfs_tag_decoder.h"

namespace CfsTag {
namespace detail {
static const uint8_t kUidKey[16] = {
  113,51,98,117,94,116,49,110,113,102,90,40,112,102,36,49
};
static const uint8_t kSectorKey[16] = {
  72,64,67,70,107,82,110,122,64,75,65,116,66,74,112,50
};
inline bool aesEcb(const uint8_t key[16], const uint8_t input[16],
                   uint8_t output[16], bool decrypt) {
  mbedtls_aes_context context;
  mbedtls_aes_init(&context);
  const int result = decrypt
      ? mbedtls_aes_setkey_dec(&context, key, 128)
      : mbedtls_aes_setkey_enc(&context, key, 128);
  const int operation = result ? result : mbedtls_aes_crypt_ecb(
      &context, decrypt ? MBEDTLS_AES_DECRYPT : MBEDTLS_AES_ENCRYPT,
      input, output);
  mbedtls_aes_free(&context);
  return operation == 0;
}
inline bool sameUid(const MFRC522::Uid &a, const uint8_t *original,
                    uint8_t originalSize) {
  return a.size == originalSize && memcmp(a.uidByte, original, originalSize) == 0;
}
} // namespace detail

// Called ONLY while a MIFARE Classic 1K card is selected. The caller always
// calls PICC_HaltA/PCD_StopCrypto1 after this function, including on failure.
// This method has no write-capable interfaces.
inline bool readMaterialSector(MFRC522 &reader, Material &material) {
  if (reader.PICC_GetType(reader.uid.sak) != MFRC522::PICC_TYPE_MIFARE_1K)
    return false;
  if (reader.uid.size != 4 && reader.uid.size != 7) return false;
  uint8_t originalUid[10] = {};
  const uint8_t uidSize = reader.uid.size;
  memcpy(originalUid, reader.uid.uidByte, uidSize);

  MFRC522::MIFARE_Key key = {};
  for (uint8_t i = 0; i < 6; ++i) key.keyByte[i] = 0xff;
  bool encrypted = false;
  MFRC522::StatusCode status = reader.PCD_Authenticate(
      MFRC522::PICC_CMD_MF_AUTH_KEY_A, 7, &key, &reader.uid);
  if (status != MFRC522::STATUS_OK) {
    reader.PCD_StopCrypto1();
    uint8_t repeatedUid[16] = {};
    for (uint8_t i=0; i<16; ++i) repeatedUid[i] = originalUid[i % 4];
    uint8_t derived[16] = {};
    if (!detail::aesEcb(detail::kUidKey, repeatedUid, derived, false)) return false;
    for (uint8_t i = 0; i < 6; ++i) key.keyByte[i] = derived[i];
    byte atqa[2] = {};
    byte atqaLength = sizeof(atqa);
    const auto wakeStatus = reader.PICC_WakeupA(atqa, &atqaLength);
    if (wakeStatus != MFRC522::STATUS_OK || !reader.PICC_ReadCardSerial() ||
        !detail::sameUid(reader.uid, originalUid, uidSize)) return false;
    status = reader.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                      7, &key, &reader.uid);
    if (status != MFRC522::STATUS_OK) return false;
    encrypted = true;
  }

  uint8_t data[48] = {};
  for (uint8_t block=4; block<=6; ++block) {
    uint8_t response[18] = {};
    byte length = sizeof(response);
    if (reader.MIFARE_Read(block, response, &length) != MFRC522::STATUS_OK ||
        length < 16) return false;
    uint8_t *destination = data + (block-4) * 16;
    if (encrypted) {
      if (!detail::aesEcb(detail::kSectorKey, response, destination, true))
        return false;
    } else {
      memcpy(destination, response, 16);
    }
  }
  return parseSector1(data, sizeof(data), material);
}
} // namespace CfsTag
