//
// kernel.cpp
//
// ILI9341 - graphical LCD 240x320, 4 wire SPI mode
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
#define SPI_MASTER_DEVICE   0
// 80 MHz
#define SPI_CLOCK_SPEED     80000000
#define SPI_CPOL            0
#define SPI_CPHA            0

// 0 or 1, or 2 (for SPI1)
#define SPI_CHIP_SELECT     0

CKernel::CKernel(void)
    : Timer(&Interrupt),
      Logger(Options.GetLogLevel(), &Timer),
#ifndef USE_SPI_MASTER_AUX
      SPIMaster(SPI_CLOCK_SPEED, SPI_CPOL, SPI_CPHA, SPI_MASTER_DEVICE),
#else
      SPIMaster(SPI_CLOCK_SPEED),
#endif
      ILI9341(&SPIMaster, SPI_CHIP_SELECT, 25)
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
        bOK = ILI9341.Initialize();
    }

    return TRUE;
}

TShutdownMode CKernel::Run(void)
{
    Logger.Write(FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);

    Logger.Write(FromKernel, LogNotice, "ILI9341 4 wire SPI mode");

    for (unsigned i = 0; i < 0xFFFF; i += 0x3F) {
        ILI9341.Paint(i);
        Logger.Write(FromKernel, LogNotice, "color %4X", i);
    }

    Logger.Write(FromKernel, LogNotice, "\nRebooting..");

    return ShutdownReboot;
}
