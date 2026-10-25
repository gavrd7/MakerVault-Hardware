MakerVault Handheld v0.5.1.13 — pre-release integration firmware

Based on physically tested v0.5.1.11 + v0.5.1.12 UI changes.
Removes temporary screenshot capture support (canvas and USB serial transport).
Pairing start over a missing/untrusted private CA now reports the underlying
TLS rejection rather than a truncated HTTPClient negative code. No insecure
fallback is permitted. Physical pairing and backend pairing-v1 are unchanged.

Arduino ESP32-S3 Dev Module, Huge APP (3MB No OTA), no Erase All Flash.
Third-party QRCode files qrcode.c and qrcode.h (ricmoo/QRCode MIT) must still
be present in sketch directory. Restore/retain working CA and keys.
Not Arduino-compiled or hardware-tested in this environment.
