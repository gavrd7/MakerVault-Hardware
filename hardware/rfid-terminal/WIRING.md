# Rev C.3 — verified ESP32-S3 handheld wiring

**Revision scope:** final Game Boy-style portrait handheld with ESP32-S3 SuperMini, MFRC522/RC522, ST7735S 1.44-inch 128×128 TFT, KY-040 encoder, Back and Confirm buttons. Pin constants checked against `firmware/rfid-terminal/MakerVault_v0523/MakerVault_v0523.ino` on the physically tested **v0.5.1.13** baseline. Avoid pinouts from earlier prototypes. The unmerged v0.5.1.14 maintenance candidate changes version strings only.

## Point-to-point connections

| Module | Module pin/signal (check your board silkscreen) | ESP32-S3 SuperMini pin | Verification / notes |
|---|---|---|---|
| RC522 | 3.3V / VCC | 3V3 | Never apply 5 V to the RC522 |
| RC522 | GND | GND | Common ground |
| RC522 | SCK | GPIO12 | Shared hardware SPI |
| RC522 | MOSI | GPIO11 | Shared hardware SPI |
| RC522 | MISO | GPIO13 | Shared hardware SPI |
| RC522 | SDA / SS / NSS | GPIO10 | Reader SPI chip select; **not I²C SDA** |
| RC522 | RST | GPIO9 | Reader reset |
| RC522 | IRQ | **Not connected** | No IRQ pin in tested sketch |
| TFT ST7735S | VCC / 3V3 | 3V3 | Check specific TFT module: voltage requirements vary |
| TFT ST7735S | GND | GND | Common ground |
| TFT ST7735S | SCL / SCK / CLK | GPIO12 | Shared SPI clock (not I²C SCL) |
| TFT ST7735S | SDA / DIN / MOSI | GPIO11 | Shared SPI MOSI (not I²C SDA) |
| TFT ST7735S | CS | GPIO1 | TFT chip select |
| TFT ST7735S | DC / A0 | GPIO2 | Command/data select |
| TFT ST7735S | RES / RST | GPIO43 | TFT hardware reset |
| TFT ST7735S | BLK / LED | 3V3 | **Prototype wiring only; current limiting not independently verified** |
| TFT ST7735S | MISO / SDO (if present) | **Not connected** | No display readback in tested build |
| KY-040 | CLK | GPIO4 | Encoder channel A, input pull-up in firmware |
| KY-040 | DT | GPIO5 | Encoder channel B, input pull-up in firmware |
| KY-040 | SW | GPIO6 | Encoder push-to-select, active-low input with pull-up |
| KY-040 | + / VCC | 3V3 | Verify module variant; do not feed ESP GPIO with 5 V |
| KY-040 | GND | GND | Common ground |
| Back button | One switched contact | GPIO7 | `INPUT_PULLUP`; closes to GND when pressed |
| Back button | Opposite switched contact | GND | Use the correct *opposite* switch terminal pair |
| Confirm button | One switched contact | GPIO8 | `INPUT_PULLUP`; closes to GND when pressed |
| Confirm button | Opposite switched contact | GND | Verify contacts with a continuity meter |

The tested sketch also instantiates `Adafruit_NeoPixel led(1,48,...)` on **GPIO48** for the on-board RGB LED. This is not an added external LED: the handheld deliberately omits discrete status LEDs. Whether GPIO48 is connected to a usable on-board LED depends on the exact SuperMini variant.

## Wiring schematic — functional, not a board footprint

```text
                         ESP32-S3 SuperMini
                      +----------------------+
   Shared SPI clock   | GPIO12 ------------+---------------- RC522 SCK
                      |                    +---------------- TFT SCL/SCK
   Shared SPI data    | GPIO11 ------------+---------------- RC522 MOSI
                      |                    +---------------- TFT SDA/DIN
                      | GPIO13 ------------------------------ RC522 MISO
   Reader select      | GPIO10 ------------------------------ RC522 SDA/SS
   Reader reset       | GPIO 9 ------------------------------ RC522 RST
   Display select     | GPIO 1 ------------------------------ TFT CS
   Display data/cmd   | GPIO 2 ------------------------------ TFT DC/A0
   Display reset      | GPIO43 ------------------------------ TFT RST
   Encoder channels   | GPIO 4 ------------------------------ KY-040 CLK
                      | GPIO 5 ------------------------------ KY-040 DT
   Encoder push       | GPIO 6 ------------------------------ KY-040 SW
   Back input         | GPIO 7 ------------------------------ Back contact
   Confirm input      | GPIO 8 ------------------------------ Confirm contact
                      | 3V3 ----+---------+---------+------- RC522 VCC
                      |         |         |         +------- TFT VCC
                      |         |         +----------------- TFT BLK*
                      |         +--------------------------- KY-040 +
                      | GND ----+---------+---------+------- RC522 GND
                      |         |         +----------------- TFT GND
                      |         +--------------------------- KY-040 GND
                      |         +---- Back/Confirm other contacts
                      +----------------------+
                                 *Prototype BLK = 3V3; see precautions.
```

**Electrical precautions:** Disconnect USB before soldering. All modules must share ground; keep SPI signals at 3.3 V. Read the silkscreen on *your exact* RC522 and TFT because “SDA/SCL” here are often SPI labels. Confirm button switch pairs with a multimeter: many 4-legged tactile switches have two internally common pins on each side, so wiring the same side will give a permanent short. The firmware enables pull-ups for the controls. Avoid routing long wires across the RC522 antenna. Isolate joints using heat-shrink; check for shorts before power-up.

**TFT backlight:** On the actual working terminal, `BLK` was wired directly to `3V3` with no external resistor. The TFT displayed images successfully, but an appropriate onboard current limiter/resistor was **not** independently confirmed. Do not generalise this connection to a different ST7735S breakout without checking its LED circuit and current specification.

**Connection verification:** The sketch declares `SCK_PIN=12`, `MOSI_PIN=11`, `MISO_PIN=13`, `TFT_CS=1`, `TFT_DC=2`, `TFT_RST=43`, `RFID_CS=10`, `RFID_RST=9`, `ENC_A=4`, `ENC_B=5`, `ENC_SW=6`, `BACK_PIN=7`, `CONFIRM_PIN=8`, and `RGB_PIN=48`. Its SPI initialisation is `SPI.begin(SCK_PIN,MISO_PIN,MOSI_PIN,-1)`. Pin labels and GPIO numbers are not interchangeable: consult the pin legend for your exact ESP32-S3 SuperMini board.
