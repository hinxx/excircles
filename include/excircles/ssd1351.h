//
// ssd1351.h
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
#ifndef _ssd1351_h
#define _ssd1351_h

#include <circle/device.h>
#include <circle/gpiopin.h>
#include <circle/types.h>
#ifndef USE_SPI_MASTER_AUX
#include <circle/spimaster.h>
#else
#include <circle/spimasteraux.h>
#endif

class SSD1351Device : public CDevice
{
public:
	SSD1351Device(CSPIMaster *_SPIMaster, unsigned _cs, unsigned _dc, unsigned _rst);
	~SSD1351Device(void);
	boolean Initialize(void);
	void WriteCommand(unsigned _cmd);
	void WriteData(unsigned _data);
	void SetXY(unsigned _x0, unsigned _x1, unsigned _y0, unsigned _y1);
	void Paint(unsigned _color);
	void Clear(void);
    void DrawPixel(unsigned _x, unsigned _y, unsigned _color);
    void DrawLine(int _x1, int _y1, int _x2, int _y2, int _color);
    void DrawSquare(unsigned _x, unsigned _y, unsigned _size, unsigned _color);
    void Spectrum(void);

private:
#ifndef USE_SPI_MASTER_AUX
	CSPIMaster *SPIMaster;
#else
	CSPIMasterAUX *SPIMaster;
#endif
    unsigned cs;
    CGPIOPin dc;
    CGPIOPin rst;
};

#endif // _ssd1351_h
