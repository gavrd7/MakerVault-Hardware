# HTTPS/TLS security validation — v0.5.1.7 experimental firmware

Candidate source: `MakerVault_Handheld_v0517_TLS_diagnostics.zip` produced alongside the development conversation. **Not compiled or verified on a physical ESP32; keep v0.5.1.6 as rollback. The full source package is not yet committed to this branch.** This is terminal-side firmware work, not MakerVault backend PR #146.

## Actual source audit

- Existing `maker_backend.h` consistently uses `WiFiClientSecure` and `HTTPClient` for pairing/start, completion, `/me/` and heartbeat.
- A populated CA PEM uses `setCACert()`. An empty CA uses `setCACertBundle(x509_crt_bundle, x509_crt_bundle_len)` (public roots). No `setInsecure()` path found in inspected firmware; HTTPS URL validated in `mvValidUrl()`.
- Therefore blank CA on a server signed by MakerVault Local CA is expected to fail, not silently trust. `HTTP: -1` alone does not prove certificate failure.
- Local QR/LAN/AP phone setup uses plaintext HTTP on the local provisioning channel; this does not change MakerVault API HTTPS verification. Do not expose provisioning webserver to untrusted networks.
- Arduino-ESP32 **3.3.12** upstream `NetworkClientSecure` provides `lastError(char*,size_t)` and `HTTPClient::errorToString(code)`; firmware diagnostics must use these, not fabricated errors.

## Candidate changes

- Single CA-selecting helper for API and verification requests, with explicit hostname lookup and timeout labels.
- On negative HTTPClient result, collect native mbedTLS error before closing the HTTPS client, preserve its text in Serial Monitor.
- Only `MBEDTLS_ERR_X509_CERT_VERIFY_FAILED` specifically becomes certificate rejection; issuer/hostname/date subcauses intentionally grouped unless underlying library returns reliable additional detail.
- HTTP 401/403/429/503 reported as API statuses, not TLS failures.
- Connection status adds trust mode, TLS result, last successful connection time, and last error. Stored pairing credential is not deleted after a TLS failure.
- **Test HTTPS** calls `/api/device/v1/me/` with current configuration without pairing or settings modification. Auth failure remains distinct from TLS failure. Retains v1 wire contract.

## Acceptance tests

- [ ] Correct Local CA gives authenticated 200
- [ ] Blank CA fails closed on privately issued server certificate (diagnostic must not claim cause without TLS proof)
- [ ] Unrelated CA fails closed
- [ ] Hostname mismatch fails closed even if underlying mbedTLS returns generic certificate verify failure
- [ ] Block TCP 8443: secure connection/timeout, not false certificate error
- [ ] Restore port: auto-reconnect without new pairing
- [ ] Reissue leaf certificate under same CA: trusted connection succeeds
- [ ] Temporary certificate error: original device credential remains in NVS
- [ ] Rejected bearer: HTTP 401/403 shown, not untrusted certificate
- [ ] Encoder/TFT partial redraw, RFID scans, pairing protocol, wifi networks, provision portal, heartbeats unchanged

**Do not merge v1.3.0 or close PR #146 based solely on this candidate.**
