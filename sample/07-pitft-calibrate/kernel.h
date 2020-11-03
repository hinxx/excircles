//
// kernel.h
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
#ifndef _kernel_h
#define _kernel_h

#include <circle/memory.h>
#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/serial.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/gpiopin.h>
#include <circle/types.h>
#ifndef USE_SPI_MASTER_AUX
#include <circle/spimaster.h>
#else
#include <circle/spimasteraux.h>
#endif
#include <excircles/ft6206.h>
#include <excircles/ili9341.h>
#include <excircles/tscalibration.h>

enum TShutdownMode
{
    ShutdownNone,
    ShutdownHalt,
    ShutdownReboot
};

class CKernel
{
public:
    CKernel(void);
    ~CKernel(void);
    boolean Initialize(void);
    TShutdownMode Run(void);

private:
    void GetSample(int _x, int _y, int _i);
    void DrawCrossHair(int x, int _y, int _c);
    void DrawHLine(int _x, int _y, int _len, int _color);
    void DrawVLine(int _x, int _y, int _len, int _color);

    void FT6206EventHandler(FT6206Event _event, unsigned _id,
                            unsigned _posX, unsigned _posY);
    static void FT6206EventHandlerStub(FT6206Event _event, unsigned _id,
                                       unsigned _posX, unsigned _posY);

private:
    // do not change this order
    CMemorySystem Memory;
    CActLED ActLED;
    CKernelOptions Options;
    CDeviceNameService DeviceNameService;
    CSerialDevice Serial;
    CExceptionHandler ExceptionHandler;
    CInterruptSystem Interrupt;
    CTimer Timer;
    CLogger Logger;
#ifndef USE_SPI_MASTER_AUX
    CSPIMaster SPIMaster;
#else
    CSPIMasterAUX SPIMaster;
#endif
    CI2CMaster I2CMaster;
    FT6206Device FT6206;
    ILI9341Device ILI9341;
    TsCalibration Calibration;
    int sampleX;
    int sampleY;
    boolean haveSample;
    static CKernel *s_pThis;
};

#endif
