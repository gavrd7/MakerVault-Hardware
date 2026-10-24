# Terminal scope-aware UI — v0.5.1.3 candidate

The local firmware candidate ZIP is `MakerVault_Handheld_v0513.zip`; firmware source is not yet part of this PR. **Not compiled or hardware-validated**. Preserve tested v0.5.1.1 as rollback.

## Requirements and implemented candidate behaviour

- Connections split into Wi-Fi and MakerVault submenus with distinct settings.
- MakerVault connection details and System info include the current device name from authenticated `GET /api/device/v1/me/` (`key_name`).
- Local **Forget pairing** requires confirmation, clears only local bearer key and device association (not Wi-Fi, CA, server URL or stable UID). Revoke the server-side device separately in MakerVault.
- Saved credentials do not independently imply a live pairing; only a successful, protocol-checked `/me/` does. 401/403 or a rejected heartbeat disable verified state without erasing the pairing key. Offline status is unknown rather than granting capabilities.
- API scopes are updated from the verified `/me/` response, allowing scopes to change server-side without re-pairing. MakerVault status lists `tags:read`, `tags:write`, `inventory:read`, and `devices:status`; rights are cleared when a check fails/offline/unpaired.
- Operations must be visible only when **both implemented in firmware and authorised by MakerVault**. Create MakerTag and Write RFID tag are not implemented, so they remain hidden regardless of `tags:write`. A granted scope is not an implementation guarantee.
- Keep general, offline UID reading accessible even without a MakerVault backend connection. Server-side operations require current scopes and must handle HTTP 403.

## Physical acceptance

Reboot; pair and verify connection/device name; change permissions in MakerVault and verify reflected on next `/me/`; revoke/delete pairing and verify UI no longer reports paired; forget locally without clearing Wi-Fi; test re-pair and reader operations. Check compiler and memory safety on Arduino-ESP32 3.x. Do not release until tested.
