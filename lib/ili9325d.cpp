//
// ili9325d.cpp
//
// ILI9325D - graphical LCD device driver
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
#include <excircles/ili9325d.h>
#include <assert.h>

static const char FromILI9325D[] = "ili9325d";

#define LCD_WIDTH               240
#define LCD_HEIGHT              320

ILI9325DDevice::ILI9325DDevice(u8 _db0, u8 _db1, u8 _db2, u8 _db3, u8 _db4, u8 _db5, u8 _db6, u8 _db7,
                     u8 _cs, u8 _wr, u8 _rs, u8 _rst)
    : db{{_db0, GPIOModeOutput},
         {_db1, GPIOModeOutput},
         {_db2, GPIOModeOutput},
         {_db3, GPIOModeOutput},
         {_db4, GPIOModeOutput},
         {_db5, GPIOModeOutput},
         {_db6, GPIOModeOutput},
         {_db7, GPIOModeOutput}},
      cs(_cs, GPIOModeOutput),
      wr(_wr, GPIOModeOutput),
      rs(_rs, GPIOModeOutput),
      rst(_rst, GPIOModeOutput)
{
//    wr.Write(HIGH);
//    rs.Write(HIGH);
//    rst.Write(HIGH);
//    cs.Write(HIGH);

//    for (unsigned i = 0; i < 8; i++) {
//        db[i].Write(HIGH);
//    }
}

ILI9325DDevice::~ILI9325DDevice(void)
{
}

boolean ILI9325DDevice::Initialize(void)
{
    // perform reset
    rst.Write(HIGH);
    CTimer::SimpleMsDelay(1);
    rst.Write(LOW);
    CTimer::SimpleMsDelay(1);
    rst.Write(HIGH);
    CTimer::SimpleMsDelay(20);

    // initialize
    WriteCommandData(0xE5, 0x78F0);
    WriteCommandData(0x01, 0x0100);
    WriteCommandData(0x02, 0x0200);
    WriteCommandData(0x03, 0x1030);
    WriteCommandData(0x04, 0x0000);
    WriteCommandData(0x08, 0x0207);
    WriteCommandData(0x09, 0x0000);
    WriteCommandData(0x0A, 0x0000);
    WriteCommandData(0x0C, 0x0000);
    WriteCommandData(0x0D, 0x0000);
    WriteCommandData(0x0F, 0x0000);
    // power on sequence
    WriteCommandData(0x10, 0x0000);
    WriteCommandData(0x11, 0x0007);
    WriteCommandData(0x12, 0x0000);
    WriteCommandData(0x13, 0x0000);
    WriteCommandData(0x07, 0x0001);
    // dis-charge capacitor power voltage
    CTimer::SimpleMsDelay(200);
    WriteCommandData(0x10, 0x1690);
    WriteCommandData(0x11, 0x0227);
    CTimer::SimpleMsDelay(50);
    WriteCommandData(0x12, 0x000D);
    CTimer::SimpleMsDelay(50);
    WriteCommandData(0x13, 0x1200);
    WriteCommandData(0x29, 0x000A);
    WriteCommandData(0x2B, 0x000D);
    CTimer::SimpleMsDelay(50);
    WriteCommandData(0x20, 0x0000);
    WriteCommandData(0x21, 0x0000);
    // adjust the gamma curve
    WriteCommandData(0x30, 0x0000);
    WriteCommandData(0x31, 0x0404);
    WriteCommandData(0x32, 0x0003);
    WriteCommandData(0x35, 0x0405);
    WriteCommandData(0x36, 0x0808);
    WriteCommandData(0x37, 0x0407);
    WriteCommandData(0x38, 0x0303);
    WriteCommandData(0x39, 0x0707);
    WriteCommandData(0x3C, 0x0504);
    WriteCommandData(0x3D, 0x0808);
    // set GRAM area
    WriteCommandData(0x50, 0x0000);
    WriteCommandData(0x51, 0x00EF);
    WriteCommandData(0x52, 0x0000);
    WriteCommandData(0x53, 0x013F);
    WriteCommandData(0x60, 0xA700);
    WriteCommandData(0x61, 0x0001);
    WriteCommandData(0x6A, 0x0000);
    // partial display control
    WriteCommandData(0x80, 0x0000);
    WriteCommandData(0x81, 0x0000);
    WriteCommandData(0x82, 0x0000);
    WriteCommandData(0x83, 0x0000);
    WriteCommandData(0x84, 0x0000);
    WriteCommandData(0x85, 0x0000);
    // panel control
    WriteCommandData(0x90, 0x0010);
    WriteCommandData(0x92, 0x0000);
    // 262K color and display ON
    WriteCommandData(0x07, 0x0133);

    WriteCommand(0x22);

    CLogger::Get()->Write(FromILI9325D, LogNotice, "ILI9325D intialized!");
    return TRUE;
}

void ILI9325DDevice::WriteCommand(unsigned _cmd)
{
    rs.Write(LOW);
    cs.Write(LOW);
    // high byte
    for (unsigned i = 0; i < 8; i++) {
        db[i].Write((_cmd & (1 << (i + 8))) ? HIGH : LOW);
    }
    wr.Write(LOW);
    wr.Write(HIGH);
    // low byte
    for (unsigned i = 0; i < 8; i++) {
        db[i].Write((_cmd & (1 << i)) ? HIGH : LOW);
    }
    wr.Write(LOW);
    wr.Write(HIGH);
    cs.Write(HIGH);
}

void ILI9325DDevice::WriteData(unsigned _data)
{
    rs.Write(HIGH);
    cs.Write(LOW);
    // high byte
    for (unsigned i = 0; i < 8; i++) {
        db[i].Write((_data & (1 << (i + 8))) ? HIGH : LOW);
    }
    wr.Write(LOW);
    wr.Write(HIGH);
    // low byte
    for (unsigned i = 0; i < 8; i++) {
        db[i].Write((_data & (1 << i)) ? HIGH : LOW);
    }
    wr.Write(LOW);
    wr.Write(HIGH);
    cs.Write(HIGH);
}

void ILI9325DDevice::WriteCommandData(unsigned _cmd, unsigned _data)
{
    WriteCommand(_cmd);
    WriteData(_data);
}

void ILI9325DDevice::SetXY(unsigned _x0, unsigned _x1, unsigned _y0, unsigned _y1)
{
    WriteCommandData(0x20, _x0);
    WriteCommandData(0x21, _y0);
    WriteCommandData(0x50, _x0);
    WriteCommandData(0x52, _y0);
    WriteCommandData(0x51, _x1);
    WriteCommandData(0x53, _y1);
    WriteCommand(0x22);
}

void ILI9325DDevice::Paint(unsigned _color)
{
    SetXY(0, LCD_WIDTH-1, 0, LCD_HEIGHT-1);
    for(unsigned i = 0; i < LCD_HEIGHT; i++) {
        for (unsigned j = 0; j < LCD_WIDTH; j++) {
            WriteData(_color);
        }
    }
}

void ILI9325DDevice::Clear(void)
{
    SetXY(0, LCD_WIDTH-1, 0, LCD_HEIGHT-1);
    for(unsigned i = 0; i < LCD_HEIGHT; i++) {
        for(unsigned j = 0; j < LCD_WIDTH; j++) {
            WriteData(0x0000);
        }
    }
}
