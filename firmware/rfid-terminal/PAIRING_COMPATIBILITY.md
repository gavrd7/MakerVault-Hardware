# Pairing compatibility

Canonical server: [MakerVault PR #146](https://github.com/gavrd7/MakerVault/pull/146), `docs/CONNECTED_DEVICES_PAIRING.md`. Protocol `makervault-device-v1`.

- `POST /api/device/v1/pairing/start/`: `device_uid`, `device_type: rfid_terminal`, fresh 32-byte base64url `claim_secret`.
- `POST /api/device/v1/pairing/{pairing_id}/complete/`: 202 pending, 200 paired one-time credential, 410 consumed/expired.
- `GET /api/device/v1/me/`: verify Bearer credential and protocol; success UI only after HTTP 200.
- `POST /api/device/v1/heartbeat/`: periodically update last-seen, with `devices:status` scope.
- Settings → Connected Devices is the **primary** onboarding method; terminal + LAN phone QR or explicit temporary AP is an alternative using the same pairing API.
- Keep manual API key as advanced/legacy only; public-trust HTTPS needs no uploaded CA; private CA trust needs explicit certificate provisioning.

Do not merge or publish backend v1.3.0 based on untested physical terminal pairing.
