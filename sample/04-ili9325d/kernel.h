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
#include <excircles/ili9325d.h>

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
    ILI9325DDevice ILI9325D;
};

#endif
