# Third-party code and asset notices

## MakerVault-owned source

MakerVault-authored source code and documentation in this repository are offered under the **GNU Affero General Public License, version 3 (AGPL-3.0)**. See [LICENSE](LICENSE).

This does not override third-party copyright or licensing.

## QRCode library — MIT

The bundled `qrcode.c` and `qrcode.h` are from [Richard Moore's QRCode](https://github.com/ricmoo/QRCode), incorporating contributions attributed to Project Nayuki, under the MIT licence.

The full notice is retained in [QRCode-LICENSE.txt](firmware/rfid-terminal/third_party/QRCode-LICENSE.txt) and must accompany redistributions. MIT-licensed source may be redistributed as part of an AGPL project while retaining its own original notice.

## Arduino library and board dependencies

The firmware uses external libraries/cores installed separately through Arduino IDE. Their licences are **not** changed by this repository's AGPL licence. See [build instructions](firmware/rfid-terminal/BUILD.md).

## Artwork and graphics — review before publication

`brand_images.h` includes an embedded RGB565 MakerVault logo derived from existing branding. `ui_icons.h` contains raster icons. **The source files do not record the original creator/licence for every graphic.** Confirm the project's rights to redistribute and license these assets (and any third-party icon sources) before switching the repository to public. Do not assume generic icon packs or modified online icons are automatically AGPL-compatible.
