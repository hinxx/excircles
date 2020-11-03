//
// ssd1351.cpp
//
// SSD1351 - OLED graphical LCD device driver
// Copyright (C) 2020  H. Kocevar <hinxx@protonmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include <circle/timer.h>
#include <circle/logger.h>
#include <excircles/ssd1351.h>
#include <assert.h>

static const char FromSSD1351[] = "ssd1351";

#define LCD_WIDTH               128
#define LCD_HEIGHT              128

#ifndef USE_SPI_MASTER_AUX
SSD1351Device::SSD1351Device(CSPIMaster *_SPIMaster, unsigned _cs, unsigned _dc, unsigned _rst)
#else
SSD1351Device::SSD1351Device(CSPIMasterAUX *_SPIMaster, unsigned _cs, unsigned _rs)
#endif
    : SPIMaster(_SPIMaster),
      cs(_cs),
      dc(_dc, GPIOModeOutput),
      rst(_rst, GPIOModeOutput)
{
    assert(_SPIMaster != 0);
}

SSD1351Device::~SSD1351Device(void)
{
    SPIMaster = 0;
}

boolean SSD1351Device::Initialize(void)
{
    // perform reset
    rst.Write(HIGH);
    CTimer::SimpleMsDelay(1);
    rst.Write(LOW);
    CTimer::SimpleMsDelay(1);
    rst.Write(HIGH);
    CTimer::SimpleMsDelay(20);

    // initialize
    WriteCommand(0xFD);
    WriteData(0x12);
    WriteCommand(0xFD);
    WriteData(0xB1);
    WriteCommand(0xAE);
    WriteCommand(0xB3);
    WriteData(0xF1);
    WriteCommand(0xCA);
    WriteData(0x7F);
    WriteCommand(0xA2);
    WriteData(0x00);
    WriteCommand(0xA1);
    WriteData(0x00);
    WriteCommand(0xA0);
    //WriteData(0xA0);
    WriteData(0xB0);
    WriteCommand(0xB5);
    WriteData(0x00);
    WriteCommand(0xAB);
    WriteData(0x01);
    WriteCommand(0xB4);
    WriteData(0xA0);
    WriteData(0xB5);
    WriteData(0x55);
    WriteCommand(0xC1);
    WriteData(0x8A);
    WriteData(0x70);
    WriteData(0x8A);
    WriteCommand(0xC7);
    WriteData(0x0F);
    WriteCommand(0xB9);
    WriteCommand(0xB1);
    WriteData(0x32);
    WriteCommand(0xBB);
    WriteData(0x07);
    WriteCommand(0xB2);
    WriteData(0xa4);
    WriteData(0x00);
    WriteData(0x00);
    WriteCommand(0xB6);
    WriteData(0x01);
    WriteCommand(0xBE);
    WriteData(0x07);
    WriteCommand(0xA6);
    WriteCommand(0xAF);

    CLogger::Get()->Write(FromSSD1351, LogNotice, "SSD1351 intialized!");
    return TRUE;
}

void SSD1351Device::WriteCommand(unsigned _cmd)
{
    dc.Write(LOW);
    u8 cmd = (u8)_cmd;
    if (SPIMaster->Write(cs, &cmd, 1) != 1) {
        CLogger::Get()->Write(FromSSD1351, LogError, "SPI write error");
    }
}

void SSD1351Device::WriteData(unsigned _data)
{
    dc.Write(HIGH);
    u8 data = (u8)_data;
    if (SPIMaster->Write(cs, &data, 1) != 1) {
        CLogger::Get()->Write(FromSSD1351, LogError, "SPI write error");
    }
}

void SSD1351Device::SetXY(unsigned _x0, unsigned _x1, unsigned _y0, unsigned _y1)
{
    // column
    WriteCommand(0x15);
    WriteData(_x0);
    WriteData(_x1);
    // row
    WriteCommand(0x75);
    WriteData(_y0);
    WriteData(_y1);
    // write
    WriteCommand(0x5C);
}

void SSD1351Device::Paint(unsigned _color)
{
    SetXY(0, LCD_WIDTH-1, 0, LCD_HEIGHT-1);
    for (unsigned i = 0; i < LCD_HEIGHT; i++) {
        for (unsigned j = 0; j < LCD_WIDTH; j++) {
            WriteData((_color >> 16) & 0xFF);
            WriteData((_color >> 8) & 0xFF);
            WriteData(_color & 0xFF);
        }
    }
}

void SSD1351Device::Clear(void)
{
    Paint(0x000000);
}

void SSD1351Device::DrawPixel(unsigned _x, unsigned _y, unsigned _color)
{
    SetXY(_x, _x, _y, _y);
    WriteData((_color >> 16) & 0xFF);
    WriteData((_color >> 8) & 0xFF);
    WriteData(_color & 0xFF);
}

void SSD1351Device::DrawLine(int _x1, int _y1, int _x2, int _y2, int _color)
{
    int deltaX = _x2-_x1 >= 0 ? _x2-_x1 : _x1-_x2;
    int signX  = _x1 < _x2 ? 1 : -1;

    int deltaY = -(_y2-_y1 >= 0 ? _y2-_y1 : _y1-_y2);
    int signY  = _y1 < _y2 ? 1 : -1;

    int error = deltaX + deltaY;

    while (1) {
        DrawPixel((unsigned)_x1, (unsigned)_y1, _color);

        if (_x1 == _x2 && _y1 == _y2) {
            break;
        }

        int error2 = error + error;
        if (error2 > deltaY) {
            error += deltaY;
            _x1 += signX;
        }

        if (error2 < deltaX) {
            error += deltaX;
            _y1 += signY;
        }
    }
}

void SSD1351Device::DrawSquare(unsigned _x, unsigned _y, unsigned _size, unsigned _color)
{
    SetXY(_x, _x + _size-1, _y, _y + _size-1);
    for (unsigned i = _x; i < _x + _size; i++) {
        for (unsigned j = _y; j < _y + _size; j++) {
            WriteData((_color >> 16) & 0xFF);
            WriteData((_color >> 8) & 0xFF);
            WriteData(_color & 0xFF);
        }
    }
}

void SSD1351Device::Spectrum(void)
{
    unsigned i, j;
    unsigned blue, green, red;

    SetXY(0, LCD_WIDTH-1, 0, 37);
    for (i = 0; i < 128; i++) {
        WriteData(0xFF); WriteData(0xFF); WriteData(0xFF);
    }
    for (i = 0; i < 36; i++) {
        blue = 0x00;
        green = 0x00;
        red = 0x3F;
        WriteData(0xFF); WriteData(0xFF); WriteData(0xFF);
        for (j = 0; j < 21; j++) {
            WriteData(blue); WriteData(green); WriteData(red);
            green += 3;
        }
        for (j = 0; j < 21; j++) {
            WriteData(blue); WriteData(green); WriteData(red);
            red -= 3;
        }
        for (j = 0; j < 21; j++) {
            WriteData(blue); WriteData(green); WriteData(red);
            blue += 3;
        }
        for (j = 0; j < 21; j++) {
            WriteData(blue); WriteData(green); WriteData(red);
            green -= 3;
        }
        for (j = 0; j < 21; j++) {
            WriteData(blue); WriteData(green); WriteData(red);
            red += 3;
        }
        for (j = 0; j < 21; j++) {
            WriteData(blue); WriteData(green); WriteData(red);
            blue -= 3;
        }
        WriteData(0xFF); WriteData(0xFF); WriteData(0xFF);
    }
    for (i = 0; i < 128; i++) {
        WriteData(0xFF); WriteData(0xFF); WriteData(0xFF);
    }
}
