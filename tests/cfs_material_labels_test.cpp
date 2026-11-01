#include <cassert>
#include <cstring>
#include "../firmware/rfid-terminal/MakerVault_Terminal_Firmware/cfs_material_labels.h"
int main() {
 const auto *esun=CfsTag::findMaterialLabel("10437");
 assert(esun && std::strcmp(esun->manufacturer,"eSUN")==0);
 assert(std::strcmp(esun->name,"PLA Basic")==0);
 assert(CfsTag::findMaterialLabel("99999")==nullptr);
 assert(CfsTag::findMaterialLabel(nullptr)==nullptr);
 assert(CfsTag::hexRgb565("E01B24")==0xE0C4);
 assert(CfsTag::hexRgb565("e01b24")==CfsTag::hexRgb565("E01B24"));
}
