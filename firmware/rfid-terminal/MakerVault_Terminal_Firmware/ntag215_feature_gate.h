#pragma once
// MakerVault Terminal v0.7 ships with confirmed NTAG215 NDEF URL writing enabled.
// Runtime permission, UID, type, CC/lock/protection, server-grant and physical
// read-back checks remain mandatory. Disable only for a read-only custom build.
// Keep the historical macro name to avoid changing the tested writer code.
#ifndef MAKERVAULT_NDEF_WRITER_EXPERIMENTAL
#define MAKERVAULT_NDEF_WRITER_EXPERIMENTAL 1
#endif
