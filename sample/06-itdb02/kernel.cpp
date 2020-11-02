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

// 0, 4, 5, 6 on Raspberry Pi 4; 0 otherwise
#define SPI_MASTER_DEVICE       0
// 5 MHz
#define SPI_CLOCK_SPEED         5000000
#define SPI_CPOL                0
#define SPI_CPHA                0
// 0 or 1, or 2 (for SPI1)
#define TSC2064_CHIP_SELECT     0
// pressure threshold to touch detect
#define TSC2064_THRESHOLD       400

CKernel *CKernel::s_pThis = 0;

CKernel::CKernel(void)
    : Timer(&Interrupt),
      Logger(Options.GetLogLevel(), &Timer),
#ifndef USE_SPI_MASTER_AUX
      SPIMaster(SPI_CLOCK_SPEED, SPI_CPOL, SPI_CPHA, SPI_MASTER_DEVICE),
#else
      SPIMaster(SPI_CLOCK_SPEED),
#endif
      TSC2046(&SPIMaster, TSC2064_CHIP_SELECT, TSC2064_THRESHOLD),
      ILI9325D(LCD_PIN_DB0, LCD_PIN_DB1, LCD_PIN_DB2, LCD_PIN_DB3,
               LCD_PIN_DB4, LCD_PIN_DB5, LCD_PIN_DB6, LCD_PIN_DB7,
               LCD_PIN_CS, LCD_PIN_WR, LCD_PIN_RS, LCD_PIN_RST),
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
        bOK = TSC2046.Initialize();
    }

    if (bOK) {
        bOK = ILI9325D.Initialize();
    }

    return TRUE;
}

TShutdownMode CKernel::Run(void)
{
    Logger.Write(FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);

    Logger.Write(FromKernel, LogNotice, "ITDB02 module touchscreen drawing test");

    TSC2046Device *pTouchScreen =
        (TSC2046Device *)DeviceNameService.GetDevice("touch1", FALSE);
    if (pTouchScreen == 0) {
        Logger.Write(FromKernel, LogPanic, "Touchscreen not found");
    }
    pTouchScreen->RegisterEventHandler(TSC2046EventHandlerStub);

    ILI9325D.Clear();
    ILI9325D.Paint(0x0000);

    // obtained with calibration
    // no rotations, no XY swap
    int coeff[7] = { -1285638, 79, 4340, 23298454, -6040, 3, 65536 };
    Calibration.SetCalibrationCoefficients(coeff, 7);

    posX = 0;
    posY = 0;

    Logger.Write(FromKernel, LogNotice, "Just use your touchscreen!");

    while (1) {
        TSC2046.Update();
        Timer.MsDelay(1000/60);
    }

    Logger.Write(FromKernel, LogNotice, "\nRebooting..");
    return ShutdownReboot;
}

void CKernel::TSC2046EventHandler(TSC2046Event _event, unsigned _id,
                                       unsigned _posX, unsigned _posY)
{
    CString message;
    int x, y;

    switch (_event)
    {
    case TSC2046EventFingerDown:
        Calibration.ApplyCalibration(_posX, _posY, &x, &y);
        message.Format("Finger #%u down at %d / %d", _id + 1, x, y);
        DrawPixel(x, y, 0xFFF0);
        posX = x;
		posY = y;
        break;
    case TSC2046EventFingerUp:
        message.Format("Finger #%u up", _id + 1);
        break;
    case TSC2046EventFingerMove:
        Calibration.ApplyCalibration(_posX, _posY, &x, &y);
        message.Format("Finger #%u moved to %d / %d", _id + 1, x, y);
        if (posX != 0 && posY != 0) {
            DrawLine(posX, posY, x, y, 0xFFF0);
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

void CKernel::TSC2046EventHandlerStub(TSC2046Event _event, unsigned _id,
                                      unsigned _posX, unsigned _posY)
{
    assert(s_pThis != 0);
    s_pThis->TSC2046EventHandler(_event, _id, _posX, _posY);
}

void CKernel::DrawPixel(int _x, int _y, int _color)
{
    ILI9325D.SetXY(_x, _x, _y, _y);
    ILI9325D.WriteData(_color);
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
