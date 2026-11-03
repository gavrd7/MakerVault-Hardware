#include <cassert>
#include "../firmware/rfid-terminal/MakerVault_Terminal_Firmware/cfs_material_labels.h"
int main() {
 // Offline RFID reading renders the tag colour without a bundled name database.
 assert(CfsTag::hexRgb565("E01B24")==0xE0C4);
 assert(CfsTag::hexRgb565("e01b24")==CfsTag::hexRgb565("E01B24"));
}
