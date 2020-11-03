//
// ili9341.cpp
//
// ILI9341 - graphical LCD device driver
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
#include <excircles/ili9341.h>
#include <assert.h>

static const char FromILI9341[] = "ili9341";

#define LCD_WIDTH               240
#define LCD_HEIGHT              320

#ifndef USE_SPI_MASTER_AUX
ILI9341Device::ILI9341Device(CSPIMaster *_SPIMaster, unsigned _cs, unsigned _rs)
#else
ILI9341Device::ILI9341Device(CSPIMasterAUX *_SPIMaster, u8 _rs)
#endif
    : SPIMaster(_SPIMaster),
      cs(_cs),
      rs(_rs, GPIOModeOutput)
{
    assert(_SPIMaster != 0);
}

ILI9341Device::~ILI9341Device(void)
{
    SPIMaster = 0;
}

boolean ILI9341Device::Initialize(void)
{
    // initialize
    WriteCommand(0x11);
    CTimer::SimpleMsDelay(20);
    WriteCommand(0x28);
    CTimer::SimpleMsDelay(5);
    WriteCommand(0xCF);
    WriteData(0x00);
    WriteData(0x83);
    WriteData(0x30);
    WriteCommand(0xED);
    WriteData(0x64);
    WriteData(0x03);
    WriteData(0x12);
    WriteData(0x81);
    WriteCommand(0xE8);
    WriteData(0x85);
    WriteData(0x01);
    WriteData(0x79);
    WriteCommand(0xCB);
    WriteData(0x39);
    WriteData(0X2C);
    WriteData(0x00);
    WriteData(0x34);
    WriteData(0x02);
    WriteCommand(0xF7);
    WriteData(0x20);
    WriteCommand(0xEA);
    WriteData(0x00);
    WriteData(0x00);
    WriteCommand(0xC0);
    WriteData(0x26);
    WriteCommand(0xC1);
    WriteData(0x11);
    WriteCommand(0xC5);
    WriteData(0x35);
    WriteData(0x3E);
    WriteCommand(0xC7);
    WriteData(0xBE);
    WriteCommand(0xB1);
    WriteData(0x00);
    WriteData(0x1B);
    WriteCommand(0xB6);
    WriteData(0x0A);
    WriteData(0x82);
    WriteData(0x27);
    WriteData(0x00);
    WriteCommand(0xB7);
    WriteData(0x07);
    WriteCommand(0x3A);
    WriteData(0x55);
    WriteCommand(0x36);
    WriteData((1<<3)|(1<<6));
    WriteCommand(0x29);
    CTimer::SimpleMsDelay(5);

    CLogger::Get()->Write(FromILI9341, LogNotice, "ILI9341 intialized!");
    return TRUE;
}

void ILI9341Device::WriteCommand(unsigned _cmd)
{
    rs.Write(LOW);
    u8 cmd = (u8)_cmd;
    if (SPIMaster->Write(cs, &cmd, 1) != 1) {
        CLogger::Get()->Write(FromILI9341, LogError, "SPI write error");
    }
}

void ILI9341Device::WriteData(unsigned _data)
{
    rs.Write(HIGH);
    u8 data = (u8)_data;
    if (SPIMaster->Write(cs, &data, 1) != 1) {
        CLogger::Get()->Write(FromILI9341, LogError, "SPI write error");
    }
}

void ILI9341Device::SetXY(unsigned _x0, unsigned _x1, unsigned _y0, unsigned _y1)
{
    // column
    WriteCommand(0x2A);
    WriteData(_x0>>8);
    WriteData(_x0);
    WriteData(_x1>>8);
    WriteData(_x1);
    // page
    WriteCommand(0x2B);
    WriteData(_y0>>8);
    WriteData(_y0);
    WriteData(_y1>>8);
    WriteData(_y1);
    // write
    WriteCommand(0x2C);
}

void ILI9341Device::Paint(unsigned _color)
{
    SetXY(0, LCD_WIDTH-1, 0, LCD_HEIGHT-1);
    for (unsigned i = 0; i < LCD_HEIGHT; i++) {
        for (unsigned j = 0; j < LCD_WIDTH; j++) {
            WriteData((_color >> 8) & 0xFF);
            WriteData(_color & 0xFF);
        }
    }
}

void ILI9341Device::Clear(void)
{
    SetXY(0, LCD_WIDTH-1, 0, LCD_HEIGHT-1);
    for (unsigned i = 0; i < LCD_HEIGHT; i++) {
        for (unsigned j = 0; j < LCD_WIDTH; j++) {
            WriteData(0x00);
            WriteData(0x00);
        }
    }
}

void ILI9341Device::Square(unsigned _x, unsigned _y, unsigned _size, unsigned _color)
{
    SetXY(_x, _x + _size-1, _y, _y + _size-1);
    for (unsigned i = _x; i < _x + _size; i++) {
        for (unsigned j = _y; j < _y + _size; j++) {
            WriteData((_color >> 8) & 0xFF);
            WriteData(_color & 0xFF);
        }
    }
}
