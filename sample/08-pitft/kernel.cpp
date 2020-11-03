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
      Calibration(240, 320, TsLibRotation180, FALSE)
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

    ILI9341.Clear();

    // LCD and TS are both in range, no calibration required
    // TS needs to be 180 deg rotated, though
    int coeff[7] = { 0, 1, 0, 0, 0, 1, 1 };
    Calibration.SetCalibrationCoefficients(coeff, 7);

    posX = 0;
    posY = 0;

    Logger.Write(FromKernel, LogNotice, "Just use your touchscreen!");

    while (1) {
        FT6206.Update();
        Timer.MsDelay(1000/60);
    }

    Logger.Write(FromKernel, LogNotice, "\nRebooting..");
    return ShutdownReboot;
}

void CKernel::FT6206EventHandler(FT6206Event _event, unsigned _id,
                                 unsigned _posX, unsigned _posY)
{
    CString message;
    int x, y;

    switch (_event)
    {
    case FT6206EventFingerDown:
        Calibration.ApplyCalibration(_posX, _posY, &x, &y);
        message.Format("Finger #%u down at %d / %d", _id + 1, x, y);
        DrawPixel(x, y, 0xfff0);
        posX = x;
		posY = y;
        break;
    case FT6206EventFingerUp:
        message.Format("Finger #%u up", _id + 1);
        break;
    case FT6206EventFingerMove:
        Calibration.ApplyCalibration(_posX, _posY, &x, &y);
        message.Format("Finger #%u moved to %d / %d", _id + 1, x, y);
        if (posX != 0 && posY != 0) {
            DrawLine(posX, posY, x, y, 0xfff0);
        }
        posX = x;
		posY = y;
        break;
    default:
        assert(0);
        break;
    }

    //Logger.Write(FromKernel, LogDebug, message);
}

void CKernel::FT6206EventHandlerStub(FT6206Event _event, unsigned _id,
                                      unsigned _posX, unsigned _posY)
{
    assert(s_pThis != 0);
    s_pThis->FT6206EventHandler(_event, _id, _posX, _posY);
}

void CKernel::DrawPixel(int _x, int _y, int _color)
{
    ILI9341.SetXY(_x, _x, _y, _y);
    ILI9341.WriteData((_color >> 8) & 0xFF);
    ILI9341.WriteData(_color & 0xFF);
}

void CKernel::DrawLine(int _x1, int _y1, int _x2, int _y2, int _color)
{
    int deltaX = _x2-_x1 >= 0 ? _x2-_x1 : _x1-_x2;
    int signX  = _x1 < _x2 ? 1 : -1;

    int deltaY = -(_y2-_y1 >= 0 ? _y2-_y1 : _y1-_y2);
    int signY  = _y1 < _y2 ? 1 : -1;

    int error = deltaX + deltaY;

    while (1) {
        DrawPixel((unsigned)_x1, (unsigned)_y1, _color);

        if (_x1 == _x2 && _y1 == _y2) {
            break;
        }

        int error2 = error + error;
        if (error2 > deltaY) {
            error += deltaY;
            _x1 += signX;
        }

        if (error2 < deltaX) {
            error += deltaX;
            _y1 += signY;
        }
    }
}
