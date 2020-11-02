//
// tsc2046.h
//
// TSC2046 - resistive touchscreen device driver
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
#ifndef _tsc2046_h
#define _tsc2046_h

#include <circle/device.h>
#include <circle/types.h>
#ifndef USE_SPI_MASTER_AUX
#include <circle/spimaster.h>
#else
#include <circle/spimasteraux.h>
#endif

enum TSC2046Event
{
    TSC2046EventFingerDown,
    TSC2046EventFingerUp,
    TSC2046EventFingerMove,
    TSC2046EventUnknown
};

typedef void TSC2046EventHandler(TSC2046Event _event, unsigned _id,
                                      unsigned _posX, unsigned _posY);

class TSC2046Device : public CDevice
{
public:
#ifndef USE_SPI_MASTER_AUX
    TSC2046Device(CSPIMaster *_SPIMaster, unsigned _cs, unsigned _threshold = 400);
#else
    CTSC2046Device(CSPIMasterAUX *_SPIMaster, unsigned _cs, unsigned _threshold = 400);
#endif
    ~TSC2046Device(void);
    boolean Initialize(void);
    // call this about 60 times per second
    void Update(void);
    void RegisterEventHandler(TSC2046EventHandler *_eventHandler);

private:
#ifndef USE_SPI_MASTER_AUX
    CSPIMaster *SPIMaster;
#else
    CSPIMasterAUX *SPIMaster;
#endif
    TSC2046EventHandler *eventHandler;
    unsigned cs;
    unsigned threshold;
    boolean touched;
    unsigned posX;
    unsigned posY;
};

#endif // _tsc2046_h
