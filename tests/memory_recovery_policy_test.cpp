#include <cassert>
#include <cstdint>
#include "../firmware/rfid-terminal/MakerVault_Terminal_Firmware/memory_recovery_policy.h"

using MakerVaultMemoryRecovery::Guard;
int main() {
  Guard guard;
  // Healthy free heap and contiguous TLS block never trigger a reboot.
  assert(!guard.check(180000, 241188, 135156, true));
  assert(!guard.check(210000, 241188, 135156, true));

  // A short-lived TLS allocation during a request must not restart the device.
  assert(!guard.check(220000, 100000, 31732, false));
  assert(!guard.check(223000, 241188, 135156, true));

  // Sustained deterioration triggers while idle, ahead of TLS exhaustion.
  assert(!guard.check(230000, 153000, 65535, true));
  assert(!guard.check(244000, 153000, 65535, true));
  assert(guard.check(245000, 153000, 65535, true));

  // A physical operation defers recovery, but does not reset the timer.
  Guard active;
  assert(!active.check(230000, 96600, 31732, false));
  assert(!active.check(265000, 96600, 31732, false));
  assert(active.check(265001, 96600, 31732, true));

  // Low memory immediately after boot cannot create an instant reset loop.
  Guard boot;
  assert(!boot.check(1000, 96600, 31732, true));
  assert(!boot.check(20000, 96600, 31732, true));
  assert(boot.check(120000, 96600, 31732, true));

  // Recovering above BOTH limits clears the hold timer.
  Guard recovered;
  assert(!recovered.check(200000, 100000, 31732, true));
  assert(!recovered.check(220000, 170000, 100000, true));
  assert(!recovered.check(225000, 100000, 31732, true));
  assert(!recovered.check(235000, 100000, 31732, true));
  assert(recovered.check(240000, 100000, 31732, true));

  // uint32_t elapsed calculation remains safe across timer wrap.
  Guard rollover;
  assert(!rollover.check(0xfffffff0UL, 96600, 31732, true));
  assert(rollover.check(200000, 96600, 31732, true));
  return 0;
}
