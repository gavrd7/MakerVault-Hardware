#pragma once
// Conservative NTAG215 identification and read-only memory safety inspection.
// This code does NOT transmit RFID commands or authorize any write operation.
#include <stddef.h>
#include <stdint.h>

namespace MakerVaultNtag215 {
struct MemoryInfo {
  bool supported;
  bool writable;
  size_t advertisedUserBytes;
};

// NTAG215 uses 7-byte UID, Type 2 capability container at page 3:
// E1 10 3E 00 (496 bytes declared in the CC). Treat other CCs as unsupported,
// not as an opportunity to guess capacity.
inline MemoryInfo inspect(const uint8_t *uid, size_t uidLength,
                          const uint8_t cc[4], const uint8_t staticLock[2],
                          const uint8_t dynamicLock[3]) {
  MemoryInfo info = {false, false, 0};
  if (!uid || !cc || !staticLock || !dynamicLock || uidLength != 7)
    return info;
  if (cc[0] != 0xE1 || cc[1] != 0x10 || cc[2] != 0x3E)
    return info;
  info.supported = true;
  info.advertisedUserBytes = static_cast<size_t>(cc[2]) * 8;
  // Only the factory-unlocked CC is supported in the first writer revision.
  // Any lock or write restriction requires a separate explicit inspection.
  const bool ccWritable = (cc[3] & 0x0F) == 0 && (cc[3] & 0xF0) == 0;
  const bool staticUnlocked = staticLock[0] == 0 && staticLock[1] == 0;
  const bool dynamicUnlocked = dynamicLock[0] == 0 &&
                                dynamicLock[1] == 0 && dynamicLock[2] == 0;
  info.writable = ccWritable && staticUnlocked && dynamicUnlocked;
  return info;
}
} // namespace MakerVaultNtag215
