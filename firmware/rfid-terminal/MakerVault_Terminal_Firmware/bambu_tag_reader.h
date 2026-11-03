#pragma once
// Experimental read-only Bambu Lab MIFARE Classic reader. Community-documented
// per-UID HKDF-SHA256 Key A (no Key B, no writes or printer interaction).
#include <MFRC522.h>
#include <mbedtls/md.h>
#include <string.h>
#include "bambu_tag_decoder.h"
namespace BambuTag {
static const uint8_t kSalt[16]={0x9a,0x75,0x9c,0xf2,0xc4,0xf7,0xca,0xff,
  0x22,0x2c,0xb9,0x76,0x9b,0x41,0xbc,0x96};
inline bool deriveKeys(const uint8_t uid[4],uint8_t keys[16][6]) {
  const mbedtls_md_info_t *md=mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
  if(!md)return false;
  uint8_t prk[32]={},t[32]={},message[40]={};
  if(mbedtls_md_hmac(md,kSalt,sizeof(kSalt),uid,4,prk)!=0)return false;
  const uint8_t info[7]={'R','F','I','D','-','A',0};
  size_t written=0,previous=0;
  for(uint8_t i=1;i<=3;i++) {
    memcpy(message,t,previous);
    memcpy(message+previous,info,sizeof(info));
    message[previous+sizeof(info)]=i;
    if(mbedtls_md_hmac(md,prk,sizeof(prk),message,
       previous+sizeof(info)+1,t)!=0)return false;
    const size_t take=(96-written)>32?32:(96-written);
    for(size_t j=0;j<take;j++) keys[(written+j)/6][(written+j)%6]=t[j];
    written+=take;previous=32;
  }
  return written==96;
}
inline bool readDocumentedTag(MFRC522 &reader,Material &out){
  if(reader.PICC_GetType(reader.uid.sak)!=MFRC522::PICC_TYPE_MIFARE_1K ||
     reader.uid.size!=4)return false;
  uint8_t keys[16][6]={};
  if(!deriveKeys(reader.uid.uidByte,keys))return false;
  uint8_t blocks[4][16]={};
  const uint8_t sectors[4]={0,0,1,1};
  const uint8_t positions[4]={1,2,4,5};
  for(size_t i=0;i<4;i++){
    if(i==0 || sectors[i]!=sectors[i-1]){
      MFRC522::MIFARE_Key key={};
      for(size_t j=0;j<6;j++)key.keyByte[j]=keys[sectors[i]][j];
      const uint8_t trailer=uint8_t(sectors[i]*4+3);
      if(reader.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
           trailer,&key,&reader.uid)!=MFRC522::STATUS_OK)return false;
    }
    uint8_t response[18]={};byte length=sizeof(response);
    if(reader.MIFARE_Read(positions[i],response,&length)!=MFRC522::STATUS_OK ||
       length<16)return false;
    memcpy(blocks[i],response,16);
  }
  return parse(blocks[0],blocks[1],blocks[2],blocks[3],out);
}
}
