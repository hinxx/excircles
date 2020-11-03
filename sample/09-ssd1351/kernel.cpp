//
// kernel.cpp
//
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
#include "kernel.h"
#include <circle/debug.h>

static const char FromKernel[] = "kernel";

// 0, 4, 5, 6 on Raspberry Pi 4; 0 otherwise
#define SPI_MASTER_DEVICE       0
// 4 MHz
#define SPI_CLOCK_SPEED         4000000
#define SPI_CPOL                0
#define SPI_CPHA                0

// 0 or 1, or 2 (for SPI1)
#define SSD1351_CHIP_SELECT     0

CKernel::CKernel(void)
    : Timer(&Interrupt),
      Logger(Options.GetLogLevel(), &Timer),
#ifndef USE_SPI_MASTER_AUX
      SPIMaster(SPI_CLOCK_SPEED, SPI_CPOL, SPI_CPHA, SPI_MASTER_DEVICE),
#else
      SPIMaster(SPI_CLOCK_SPEED),
#endif
      SSD1351(&SPIMaster, SSD1351_CHIP_SELECT, 24, 25)
{
    // show we are alive
    ActLED.Blink(5);
}

CKernel::~CKernel(void)
{
}

boolean CKernel::Initialize(void)
{
    boolean bOK = TRUE;

    if (bOK) {
        bOK = Serial.Initialize(115200);
    }

    if (bOK) {
        bOK = Logger.Initialize(&Serial);
    }

    if (bOK) {
        bOK = Interrupt.Initialize();
    }

    if (bOK) {
        bOK = Timer.Initialize();
    }

    if (bOK) {
        bOK = SPIMaster.Initialize();
    }

    if (bOK) {
        bOK = SSD1351.Initialize();
    }

    return TRUE;
}

TShutdownMode CKernel::Run(void)
{
    Logger.Write(FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);

    Logger.Write(FromKernel, LogNotice, "SSD1351 4 wire SPI mode");

    SSD1351.Clear();

    CTimer::SimpleMsDelay(500);
    SSD1351.Paint(0xF80000);
    CTimer::SimpleMsDelay(500);
    SSD1351.Paint(0x007e00);
    CTimer::SimpleMsDelay(500);
    SSD1351.Paint(0x00001f);
    CTimer::SimpleMsDelay(500);
    SSD1351.Paint(0xffffff);
    CTimer::SimpleMsDelay(500);
    SSD1351.Paint(0x000000);

    unsigned size = 8;
    unsigned color = 0x00FFFF;
    unsigned x = 8;
    unsigned y = 8;
    Logger.Write(FromKernel, LogNotice, "square at %3d, %3d", x, y);
    SSD1351.DrawSquare(x, y, size, color);
    CTimer::SimpleMsDelay(1000);
    SSD1351.DrawSquare(x, y, size, 0x0000);
    x = 112;
    y = 8;
    Logger.Write(FromKernel, LogNotice, "square at %3d, %3d", x, y);
    SSD1351.DrawSquare(x, y, size, color);
    CTimer::SimpleMsDelay(1000);
    SSD1351.DrawSquare(x, y, size, 0x0000);
    x = 112;
    y = 112;
    Logger.Write(FromKernel, LogNotice, "square at %3d, %3d", x, y);
    SSD1351.DrawSquare(x, y, size, color);
    CTimer::SimpleMsDelay(1000);
    SSD1351.DrawSquare(x, y, size, 0x0000);
    x = 8;
    y = 112;
    Logger.Write(FromKernel, LogNotice, "square at %3d, %3d", x, y);
    SSD1351.DrawSquare(x, y, size, color);
    CTimer::SimpleMsDelay(1000);
    SSD1351.DrawSquare(x, y, size, 0x0000);
    x = 60;
    y = 60;
    Logger.Write(FromKernel, LogNotice, "square at %3d, %3d", x, y);
    SSD1351.DrawSquare(x, y, size, color);
    CTimer::SimpleMsDelay(1000);
    SSD1351.DrawSquare(x, y, size, 0x0000);

    for (int i = 10; i < 100; i++) {
        SSD1351.DrawPixel(i, 10, 0xFF0000);
    }
    CTimer::SimpleMsDelay(1000);

    for (int i = 10; i < 100; i++) {
        SSD1351.DrawPixel(i, 30, 0x00FF00);
    }
    CTimer::SimpleMsDelay(1000);

    for (int i = 10; i < 100; i++) {
        SSD1351.DrawPixel(i, 50, 0x0000FF);
    }
    CTimer::SimpleMsDelay(1000);

    SSD1351.DrawLine(10, 10, 100, 100, 0xFF00FF);
    SSD1351.DrawLine(10, 100, 100, 10, 0xFFFF00);
    CTimer::SimpleMsDelay(1000);

    SSD1351.Clear();
    SSD1351.Spectrum();
    CTimer::SimpleMsDelay(2000);


    Logger.Write(FromKernel, LogNotice, "\nRebooting..");

    return ShutdownReboot;
}
