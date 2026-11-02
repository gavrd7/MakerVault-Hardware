#pragma once
// Read-only CFS material-spool association by physical RFID UID.
// GET only; no tag writes, printer access, or inventory mutations.
String cfsLookupUid, cfsLookupSpool, cfsLookupError;
String cfsLookupMaterialId, cfsMaterialName, cfsMaterialManufacturer;
volatile uint8_t cfsMaterialLookupState = 0; // 0 idle, 1 checking, 2 matched, 3 unknown, 4 unavailable
volatile uint8_t cfsLookupState = 0; // 0 idle, 1 checking, 2 matched, 3 unmatched, 4 error
bool cfsLookupQueued = false;
uint32_t cfsLookupSerial = 0;

void cfsSpoolLookupTask(void *) {
  const uint32_t serial = cfsLookupSerial;
  const String uid = cfsLookupUid;
  // Resolve the decoded material ID independently of the physical spool UID.
  // Do not turn a material match into an inventory association.
  if (cfsLookupMaterialId.length() == 5) {
    String materialResponse;
    const int materialStatus = mvHttps("/api/device/v1/cfs/material/?id=" + cfsLookupMaterialId,
                                      "GET", "", materialResponse, true);
    if (serial != cfsLookupSerial) {
      mvTask=nullptr;
      vTaskDelete(nullptr);
      return;
    }
    if (materialStatus == 200 && materialResponse.indexOf("\\"found\\": true") >= 0) {
      cfsMaterialManufacturer = mvField(materialResponse, "manufacturer");
      cfsMaterialName = mvField(materialResponse, "name");
      cfsMaterialLookupState = cfsMaterialName.length() ? 2 : 3;
    } else if (materialStatus == 200) {
      cfsMaterialLookupState = 3;
    } else {
      cfsMaterialLookupState = 4;
    }
  }
  String response;
  const int status = mvHttps("/api/device/v1/spools/by-rfid/?uid=" + uid,
                             "GET", "", response, true);
  if (serial != cfsLookupSerial) {
    mvTask=nullptr;
    vTaskDelete(nullptr);
    return;
  }
  if (status == 200 && response.indexOf("\"found\": true") >= 0) {
    cfsLookupSpool = mvField(response, "spool_id");
    cfsLookupState = cfsLookupSpool.length() ? 2 : 4;
  } else if (status == 200 && response.indexOf("\"found\": false") >= 0) {
    cfsLookupState = 3;
  } else {
    cfsLookupError = status == 403 ? "Access denied"
                    : status == 401 ? "Pair again" : "Lookup unavailable";
    cfsLookupState = 4;
  }
  mvTask=nullptr;
  vTaskDelete(nullptr);
}
void cfsSpoolLookupBegin(const String &uid, const String &materialId) {
  ++cfsLookupSerial;
  cfsLookupUid=uid;
  cfsLookupMaterialId=materialId;
  cfsMaterialName=""; cfsMaterialManufacturer="";
  cfsMaterialLookupState=1;
  cfsLookupSpool="";
  cfsLookupError="";
  cfsLookupState=1;
  cfsLookupQueued=true;
}
void cfsSpoolLookupDispatch() {
  if (!cfsLookupQueued || mvTask || cfsLookupState != 1) return;
  if (!mvPairVerified || !mvEndpointApproved() || WiFi.status() != WL_CONNECTED) {
    cfsLookupQueued=false;
    cfsLookupError="Lookup unavailable";
    cfsLookupState=4;
    cfsMaterialLookupState=4;
    return;
  }
  cfsLookupQueued=false;
  if (xTaskCreatePinnedToCore(cfsSpoolLookupTask, "cfs-spool-read",
      10240, nullptr, 1, &mvTask, 0) != pdPASS) {
    mvTask=nullptr;
    cfsLookupError="Task unavailable";
    cfsLookupState=4;
    cfsMaterialLookupState=4;
  }
}
