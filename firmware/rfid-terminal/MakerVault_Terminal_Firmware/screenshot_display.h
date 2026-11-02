#pragma once
// Optional, read-only USB screenshot mirror. No RFID / network operations.
// RGB565 is tracked in logical 128x128 display coordinates.
#include <Adafruit_ST7735.h>
#include <string.h>
#ifndef MAKERVAULT_SCREENSHOT_ENABLED
#define MAKERVAULT_SCREENSHOT_ENABLED 1
#endif
class MakerVaultScreenshotDisplay : public Adafruit_ST7735 {
public:
  MakerVaultScreenshotDisplay(SPIClass *spi,int8_t cs,int8_t dc,int8_t rst)
    : Adafruit_ST7735(spi,cs,dc,rst) { memset(pixels,0,sizeof(pixels)); }
#if MAKERVAULT_SCREENSHOT_ENABLED
  uint16_t pixels[128*128];
  void drawPixel(int16_t x,int16_t y,uint16_t color) override {
    if(x>=0 && x<128 && y>=0 && y<128) pixels[y*128+x]=color;
    Adafruit_ST7735::drawPixel(x,y,color);
  }
  void writePixel(int16_t x,int16_t y,uint16_t color) override {
    if(x>=0 && x<128 && y>=0 && y<128) pixels[y*128+x]=color;
    Adafruit_ST7735::writePixel(x,y,color);
  }
  void writeFillRect(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t color) override {
    const int16_t l=x>0?x:0, t=y>0?y:0;
    const int16_t r=(int32_t(x)+w)<128?(int32_t(x)+w):128;
    const int16_t b=(int32_t(y)+h)<128?(int32_t(y)+h):128;
    for(int16_t yy=t;yy<b;yy++)for(int16_t xx=l;xx<r;xx++)pixels[yy*128+xx]=color;
    Adafruit_ST7735::writeFillRect(x,y,w,h,color);
  }
  void writeFastHLine(int16_t x,int16_t y,int16_t w,uint16_t color) override {
    writeFillRectMirror(x,y,w,1,color);
    Adafruit_ST7735::writeFastHLine(x,y,w,color);
  }
  void writeFastVLine(int16_t x,int16_t y,int16_t h,uint16_t color) override {
    writeFillRectMirror(x,y,1,h,color);
    Adafruit_ST7735::writeFastVLine(x,y,h,color);
  }
  void writeFillRectMirror(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t color) {
    const int16_t l=x>0?x:0,t=y>0?y:0;
    const int16_t r=(int32_t(x)+w)<128?(int32_t(x)+w):128;
    const int16_t b=(int32_t(y)+h)<128?(int32_t(y)+h):128;
    for(int16_t yy=t;yy<b;yy++)for(int16_t xx=l;xx<r;xx++)pixels[yy*128+xx]=color;
  }
  void screenshotTo(Stream &stream) const {
    // Framing: ASCII MVSHOT1 + 32768 bytes, RGB565 little endian.
    stream.write(reinterpret_cast<const uint8_t *>("MVSHOT1"),7);
    for(size_t i=0;i<128*128;i++) {
      const uint8_t bytes[2]={uint8_t(pixels[i]&255),uint8_t(pixels[i]>>8)};
      stream.write(bytes,2);
    }
  }
#endif
};
