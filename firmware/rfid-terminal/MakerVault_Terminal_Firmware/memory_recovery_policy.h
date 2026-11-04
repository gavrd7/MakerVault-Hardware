#pragma once
#include <stddef.h>
#include <stdint.h>

// Emergency recovery only. This does not correct the cause of retained TLS
// allocations. Measure after a request finishes, never during its handshake.
namespace MakerVaultMemoryRecovery {
constexpr size_t kMinFreeInternal = 150U * 1024U;
constexpr size_t kMinLargestBlock = 64U * 1024U;
constexpr uint32_t kLowMemoryHoldMs = 15000UL;
constexpr uint32_t kMinBootUptimeMs = 120000UL;
constexpr uint32_t kCheckIntervalMs = 5000UL;

struct Guard {
  bool lowSeen = false;
  uint32_t firstLowAt = 0;

  bool check(uint32_t now, size_t freeInternal, size_t largestBlock,
             bool safeToRestart) {
    const bool low = freeInternal < kMinFreeInternal ||
                     largestBlock < kMinLargestBlock;
    if (!low) {
      lowSeen = false;
      firstLowAt = 0;
      return false;
    }
    if (!lowSeen) {
      lowSeen = true;
      firstLowAt = now;
      return false;
    }
    // A blocked physical operation does not clear the low-memory interval:
    // restart promptly once it has finished, if memory is still low.
    return safeToRestart && now >= kMinBootUptimeMs &&
           static_cast<uint32_t>(now - firstLowAt) >= kLowMemoryHoldMs;
  }
};
}  // namespace MakerVaultMemoryRecovery
