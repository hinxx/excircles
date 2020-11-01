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

static const char FromKernel[] = "kernel";

// define 8 bit data bus lines
#define LCD_PIN_DB0         21
#define LCD_PIN_DB1         20
#define LCD_PIN_DB2         26
#define LCD_PIN_DB3         16
#define LCD_PIN_DB4         19
#define LCD_PIN_DB5         13
#define LCD_PIN_DB6         12
#define LCD_PIN_DB7         6
// define control lines
#define LCD_PIN_CS          22
#define LCD_PIN_WR          1
#define LCD_PIN_RS          5
#define LCD_PIN_RST         23

CKernel::CKernel(void)
    : Timer(&Interrupt),
      Logger(Options.GetLogLevel(), &Timer),
      ILI9325D(LCD_PIN_DB0, LCD_PIN_DB1, LCD_PIN_DB2, LCD_PIN_DB3,
               LCD_PIN_DB4, LCD_PIN_DB5, LCD_PIN_DB6, LCD_PIN_DB7,
               LCD_PIN_CS, LCD_PIN_WR, LCD_PIN_RS, LCD_PIN_RST)
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
        CDevice *pTarget = DeviceNameService.GetDevice(Options.GetLogDevice(), FALSE);
        if (pTarget == 0) {
            pTarget = &Serial;
        }
        bOK = Logger.Initialize(pTarget);
    }

    if (bOK) {
        bOK = Interrupt.Initialize();
    }

    if (bOK) {
        bOK = Timer.Initialize();
    }

    if (bOK) {
        bOK = ILI9325D.Initialize();
    }

    return TRUE;
}

TShutdownMode CKernel::Run(void)
{
    Logger.Write(FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);

    Logger.Write(FromKernel, LogNotice, "ILI9325D 8-bit mode");

    ILI9325D.Clear();

    CTimer::SimpleMsDelay(500);
    ILI9325D.Paint(0xF800);
    CTimer::SimpleMsDelay(500);
    ILI9325D.Paint(0x07e0);
    CTimer::SimpleMsDelay(500);
    ILI9325D.Paint(0x001f);
    CTimer::SimpleMsDelay(500);
    ILI9325D.Paint(0xffff);
    CTimer::SimpleMsDelay(500);
    ILI9325D.Paint(0x0000);

    Logger.Write(FromKernel, LogNotice, "\nRebooting..");
    return ShutdownReboot;
}
