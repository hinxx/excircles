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

// 0, 4, 5, 6 on Raspberry Pi 4; 0 otherwise
#define SPI_MASTER_DEVICE       0
// 5 MHz
#define SPI_CLOCK_SPEED         5000000
#define SPI_CPOL                0
#define SPI_CPHA                0
// 0 or 1, or 2 (for SPI1)
#define TSC2064_CHIP_SELECT     0
// pressure threshold to touch detect
#define TSC2064_THRESHOLD       128

static const char FromKernel[] = "kernel";

CKernel *CKernel::s_pThis = 0;

CKernel::CKernel(void)
    : Timer(&Interrupt),
      Logger(Options.GetLogLevel(), &Timer),
#ifndef USE_SPI_MASTER_AUX
      SPIMaster(SPI_CLOCK_SPEED, SPI_CPOL, SPI_CPHA, SPI_MASTER_DEVICE),
#else
      SPIMaster(SPI_CLOCK_SPEED),
#endif
      TSC2046(&SPIMaster, TSC2064_CHIP_SELECT, TSC2064_THRESHOLD)
{
    s_pThis = this;
    // show we are alive
    ActLED.Blink(5);
}

CKernel::~CKernel(void)
{
    s_pThis = 0;
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
        bOK = SPIMaster.Initialize();
    }

    if (bOK) {
        bOK = TSC2046.Initialize();
    }

	return bOK;
}

TShutdownMode CKernel::Run(void)
{
    Logger.Write(FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);

    Logger.Write(FromKernel, LogNotice, "TSC2048/XPT2048 over SPI");

    TSC2046Device *pTouchScreen =
        (TSC2046Device *)DeviceNameService.GetDevice("touch1", FALSE);
    if (pTouchScreen == 0) {
        Logger.Write(FromKernel, LogPanic, "Touchscreen not found");
    }
    pTouchScreen->RegisterEventHandler(TSC2046EventHandlerStub);

    Logger.Write(FromKernel, LogNotice, "Just use your touchscreen!");

    while (1) {
        pTouchScreen->Update();
        Timer.MsDelay(1000/60);
    }

    return ShutdownReboot;
}

void CKernel::TSC2046EventHandler(TSC2046Event _event, unsigned _id,
                                       unsigned _posX, unsigned _posY)
{
    CString message;

    switch (_event)
    {
    case TSC2046EventFingerDown:
        message.Format("Finger #%u down at %u / %u", _id + 1, _posX, _posY);
        break;
    case TSC2046EventFingerUp:
        message.Format("Finger #%u up", _id + 1);
        break;
    case TSC2046EventFingerMove:
        message.Format("Finger #%u moved to %u / %u", _id + 1, _posX, _posY);
        break;
    default:
        assert(0);
        break;
    }

    Logger.Write(FromKernel, LogNotice, message);
}

void CKernel::TSC2046EventHandlerStub(TSC2046Event _event, unsigned _id,
                                          unsigned _posX, unsigned _posY)
{
    assert(s_pThis != 0);
    s_pThis->TSC2046EventHandler(_event, _id, _posX, _posY);
}
