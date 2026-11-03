#pragma once
// Clean only the 496-byte NDEF user data region (pages 4..127) of an
// authenticated, supported, unlocked NTAG215. Keep UID, CC, locks, OTP,
// configuration, password/PACK and MakerVault server records unchanged.
#include "ntag215_page_transaction.h"

namespace MakerVaultNtag215 {
enum CleanResult {
  CLEAN_VERIFIED,
  CLEAN_REJECTED,
  CLEAN_READ_FAILED,
  CLEAN_WRITE_FAILED,
  CLEAN_VERIFY_FAILED
};

inline bool emptyUserData(const uint8_t *data, size_t size) {
  if (!data || size != kUserBytes) return false;
  for (size_t i=0; i<size; ++i) if (data[i]) return false;
  return true;
}

// A verified blank result is the only successful outcome. Caller must enforce
// live server authorisation, exact UID, physical confirmation, and a fresh
// re-selection/read of the SAME UID before reporting success to MakerVault.
// This function never edits a MakerVault record.
inline CleanResult cleanUserData(PageIO &io, const uint8_t uid[7],
                                 const char *expectedUid,
                                 bool confirmed, bool authorized) {
  if (!confirmed || !authorized || !matchesUid(uid,7,expectedUid))
    return CLEAN_REJECTED;
  MemoryInfo info={false,false,0};
  if (!inspectMemory(io,uid,info)) return CLEAN_READ_FAILED;
  if (!info.supported || !info.writable || info.advertisedUserBytes!=kUserBytes)
    return CLEAN_REJECTED;
  uint8_t original[kUserBytes]={};
  if (!readNdefArea(io,original)) return CLEAN_READ_FAILED;
  // Restrict cleaning to a recognised MakerVault NFC URI, or an already blank
  // tag. Arbitrary foreign NDEF data must not be silently destroyed.
  if (!blankNdef(original,sizeof(original))) {
    char uri[MakerVaultNdef::kMaxUriLength+1]={};
    if (!MakerVaultNdef::decodeUri(original,sizeof(original),uri,sizeof(uri)) ||
        !MakerVaultNdef::validUri(uri,strlen(uri)))
      return CLEAN_REJECTED;
  }
  static const uint8_t zeros[4]={0,0,0,0};
  // Invalidate the NDEF TLV first, making an interrupted clean unreadable as
  // a valid NDEF message. All remaining user-data pages are then erased.
  if (!io.writePage(kFirstUserPage,zeros)) return CLEAN_WRITE_FAILED;
  for (size_t p=1;p<kUserBytes/4;++p)
    if (!io.writePage(uint8_t(kFirstUserPage+p),zeros))
      return CLEAN_WRITE_FAILED;
  uint8_t readback[kUserBytes]={};
  if (!readNdefArea(io,readback)) return CLEAN_VERIFY_FAILED;
  return emptyUserData(readback,sizeof(readback))
    ? CLEAN_VERIFIED : CLEAN_VERIFY_FAILED;
}
} // namespace MakerVaultNtag215
