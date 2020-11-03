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

// 0, 4, 5, 6 on Raspberry Pi 4; 0 otherwise
#define SPI_MASTER_DEVICE       0
// 5 MHz
#define SPI_CLOCK_SPEED         80000000
#define SPI_CPOL                0
#define SPI_CPHA                0
// 0 or 1, or 2 (for SPI1)
#define ILI9341_CHIP_SELECT     0

// 0 on Raspberry Pi 1 Rev. 1 boards, 1 otherwise
#define I2C_MASTER_DEVICE       1
// 0 or 1 on Raspberry Pi 4, 0 otherwise
#define I2C_MASTER_CONFIG       0
// standard mode (100 Kbps) or fast mode (400 Kbps)
#define I2C_FAST_MODE           FALSE
// pressure threshold to touch detect
#define FT6206_THRESHOLD        400

CKernel *CKernel::s_pThis = 0;

CKernel::CKernel(void)
    : Timer(&Interrupt),
      Logger(Options.GetLogLevel(), &Timer),
#ifndef USE_SPI_MASTER_AUX
      SPIMaster(SPI_CLOCK_SPEED, SPI_CPOL, SPI_CPHA, SPI_MASTER_DEVICE),
#else
      SPIMaster(SPI_CLOCK_SPEED),
#endif
      I2CMaster(I2C_MASTER_DEVICE, I2C_FAST_MODE, I2C_MASTER_CONFIG),
      FT6206(&I2CMaster),
      ILI9341(&SPIMaster, ILI9341_CHIP_SELECT, 25),
      Calibration(240, 320, 0, FALSE)
{
    s_pThis = this;
    // show we are alive
    ActLED.Blink(5);
}

CKernel::~CKernel(void)
{
    s_pThis = 0;
    SPIMaster = 0;
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
        bOK = I2CMaster.Initialize();
    }

    if (bOK) {
        bOK = FT6206.Initialize();
    }

    if (bOK) {
        bOK = ILI9341.Initialize();
    }

    return TRUE;
}

TShutdownMode CKernel::Run(void)
{
    Logger.Write(FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);

    Logger.Write(FromKernel, LogNotice, "PiTFT module touchscreen drawing test");

    FT6206Device *pTouchScreen =
        (FT6206Device *)DeviceNameService.GetDevice("touch1", FALSE);
    if (pTouchScreen == 0) {
        Logger.Write(FromKernel, LogPanic, "Touchscreen not found");
    }
    pTouchScreen->RegisterEventHandler(FT6206EventHandlerStub);
    Logger.Write(FromKernel, LogNotice, "Calibrating touchscreen!");

    ILI9341.Clear();

    // 5 points
    GetSample(20, 20, 0);
    Timer.SimpleMsDelay(1000);
    GetSample(220, 20, 1);
    Timer.SimpleMsDelay(1000);
    GetSample(220, 300, 2);
    Timer.SimpleMsDelay(1000);
    GetSample(20, 300, 3);
    Timer.SimpleMsDelay(1000);
    GetSample(120, 160, 4);
    Calibration.PerformCalibration();

    Timer.SimpleMsDelay(1000);
    Logger.Write(FromKernel, LogNotice, "\nRebooting..");

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
        sampleX = _posX;
        sampleY = _posY;
        break;
    case FT6206EventFingerUp:
        message.Format("Finger #%u up", _id + 1);
        haveSample = TRUE;
        break;
    case FT6206EventFingerMove:
        message.Format("Finger #%u moved to %u / %u", _id + 1, _posX, _posY);
        sampleX = _posX;
        sampleY = _posY;
        break;
    default:
        assert(0);
        break;
    }

    Logger.Write(FromKernel, LogDebug, message);
}

void CKernel::FT6206EventHandlerStub(FT6206Event _event, unsigned _id,
                                     unsigned _posX, unsigned _posY)
{
    assert(s_pThis != 0);
    s_pThis->FT6206EventHandler(_event, _id, _posX, _posY);
}

void CKernel::GetSample(int _x, int _y, int _i)
{
    Logger.Write(FromKernel, LogNotice, "Calibration point %d at %3d, %3d", _i, _x, _y);

    haveSample = FALSE;
    sampleX = 0;
    sampleY = 0;
    DrawCrossHair(_x, _y, 0x0FF0);

    while (1) {
        FT6206.Update();
        Timer.MsDelay(1000/60);
        if (sampleX && sampleY && haveSample) {
            break;
        }
    }

    haveSample = FALSE;
    DrawCrossHair(_x, _y, 0xFF00);
    Calibration.AddCalibrationPoint(_i, _x, _y, sampleX, sampleY);
}

void CKernel::DrawCrossHair(int _x, int _y, int _c)
{
    DrawHLine(_x-5, _y, 10, _c);
    DrawVLine(_x, _y-5, 10, _c);
}

void CKernel::DrawHLine(int _x, int _y, int _len, int _color)
{
    ILI9341.SetXY(_x, _x + _len, _y, _y);
    for (int i = 0; i < _len + 1; i++) {
        ILI9341.WriteData((_color >> 8) & 0xFF);
        ILI9341.WriteData(_color & 0xFF);
    }
}

void CKernel::DrawVLine(int _x, int _y, int _len, int _color)
{
    ILI9341.SetXY(_x, _x, _y, _y + _len);
    for (int i = 0; i < _len + 1; i++) {
        ILI9341.WriteData((_color >> 8) & 0xFF);
        ILI9341.WriteData(_color & 0xFF);
    }
}
