//
// kernel.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
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
#include <assert.h>

// 0 on Raspberry Pi 1 Rev. 1 boards, 1 otherwise
#define I2C_MASTER_DEVICE       1
// 0 or 1 on Raspberry Pi 4, 0 otherwise
#define I2C_MASTER_CONFIG       0
// standard mode (100 Kbps) or fast mode (400 Kbps)
#define I2C_FAST_MODE           FALSE

static const char FromKernel[] = "kernel";

CKernel *CKernel::s_pThis = 0;

CKernel::CKernel(void)
    : Timer(&Interrupt),
      Logger(Options.GetLogLevel(), &Timer),
      I2CMaster(I2C_MASTER_DEVICE, I2C_FAST_MODE, I2C_MASTER_CONFIG),
      FT6206(&I2CMaster)
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
        bOK = I2CMaster.Initialize();
    }

    if (bOK) {
        bOK = FT6206.Initialize();
    }

    return bOK;
}

TShutdownMode CKernel::Run(void)
{
    Logger.Write(FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);

    FT6206Device *pTouchScreen =
        (FT6206Device *)DeviceNameService.GetDevice("touch1", FALSE);
    if (pTouchScreen == 0) {
        Logger.Write(FromKernel, LogPanic, "Touchscreen not found");
    }
    pTouchScreen->RegisterEventHandler(FT6206EventHandlerStub);

    Logger.Write(FromKernel, LogNotice, "Just use your touchscreen!");

    while (1) {
        pTouchScreen->Update();
        Timer.MsDelay(1000/60);
    }

    return ShutdownReboot;
}

void CKernel::FT6206EventHandler(FT6206Event _event, unsigned _id,
                                 unsigned _posX, unsigned _posY)
{
    CString message;

    switch (_event)
    {
    case FT6206EventFingerDown:
        message.Format("Finger #%u down at %u / %u", _id + 1, _posX, _posY);
        break;
    case FT6206EventFingerUp:
        message.Format("Finger #%u up", _id + 1);
        break;
    case FT6206EventFingerMove:
        message.Format("Finger #%u moved to %u / %u", _id + 1, _posX, _posY);
        break;
    default:
        assert(0);
        break;
    }

    Logger.Write(FromKernel, LogNotice, message);
}

void CKernel::FT6206EventHandlerStub(FT6206Event _event, unsigned _id,
                                     unsigned _posX, unsigned _posY)
{
    assert(s_pThis != 0);
    s_pThis->FT6206EventHandler(_event, _id, _posX, _posY);
}
