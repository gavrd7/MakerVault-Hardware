#include <cassert>
#include <cstdint>
#include <cstring>
#include "../firmware/rfid-terminal/MakerVault_Terminal_Firmware/cfs_tag_decoder.h"

int main() {
  uint8_t data[48] = {};
  const char *sample = "AB1240276A210100100000FF016500000100000000000000";
  static_assert(sizeof(data) == 48, "sector length");
  std::memcpy(data, sample, 48);
  CfsTag::Material m = {};
  assert(CfsTag::parseSector1(data, sizeof(data), m));
  assert(std::strcmp(m.materialId, "01001") == 0);
  assert(std::strcmp(m.color, "FF0165") == 0);
  assert(std::strcmp(CfsTag::weightLabel("0330"), "1 KG") == 0);
  assert(!CfsTag::parseSector1(data, 47, m));
  data[0]='X'; assert(!CfsTag::parseSector1(data,48,m)); data[0]='A';
  data[19]='Z'; assert(!CfsTag::parseSector1(data,48,m));
  assert(!CfsTag::parseSector1(nullptr,48,m));
  return 0;
}
