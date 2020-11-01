//
// ili9325d.h
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
#ifndef _ili9325d_h
#define _ili9325d_h

#include <circle/device.h>
#include <circle/gpiopin.h>
#include <circle/types.h>

class ILI9325DDevice : public CDevice
{
public:
    ILI9325DDevice(u8 _db0, u8 _db1, u8 _db2, u8 _db3, u8 _db4, u8 _db5, u8 _db6, u8 _db7,
                   u8 _cs, u8 _wr, u8 _rs, u8 _rst);
    ~ILI9325DDevice(void);
    boolean Initialize(void);
    void WriteCommand(unsigned _cmd);
    void WriteData(unsigned _data);
    void WriteCommandData(unsigned _cmd, unsigned _data);
    void SetXY(unsigned _x0, unsigned _x1, unsigned _y0, unsigned _y1);
    void Paint(unsigned _color);
    void Clear(void);

private:
    CGPIOPin db[8];
    CGPIOPin cs;
    CGPIOPin wr;
    CGPIOPin rs;
    CGPIOPin rst;
};

#endif // _ili9325d_h
