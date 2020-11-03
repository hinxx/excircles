//
// ili9341.h
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
#ifndef _ili9341_h
#define _ili9341_h

#include <circle/device.h>
#include <circle/gpiopin.h>
#include <circle/types.h>
#ifndef USE_SPI_MASTER_AUX
#include <circle/spimaster.h>
#else
#include <circle/spimasteraux.h>
#endif

class ILI9341Device : public CDevice
{
public:
	ILI9341Device(CSPIMaster *_SPIMaster, unsigned _cs, unsigned _rs);
	~ILI9341Device(void);
	boolean Initialize(void);
	void WriteCommand(unsigned _cmd);
	void WriteData(unsigned _data);
	void SetXY(unsigned _x0, unsigned _x1, unsigned _y0, unsigned _y1);
	void Paint(unsigned _color);
	void Clear(void);
    void Square(unsigned _x, unsigned _y, unsigned _size, unsigned _color);

private:
#ifndef USE_SPI_MASTER_AUX
	CSPIMaster *SPIMaster;
#else
	CSPIMasterAUX *SPIMaster;
#endif
    unsigned cs;
	CGPIOPin rs;
};

#endif // _ili9341_h
