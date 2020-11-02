//
// ft6206.h
//
// FT6206 - capacitive touchscreen device driver
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
#ifndef _ft6206_h
#define _ft6206_h

#include <circle/device.h>
#include <circle/types.h>
#include <circle/i2cmaster.h>

enum FT6206Event
{
    FT6206EventFingerDown,
    FT6206EventFingerUp,
    FT6206EventFingerMove,
    FT6206EventUnknown
};

typedef void TSC2046EventHandler(FT6206Event _event, unsigned _id,
                                 unsigned _posX, unsigned _posY);

class FT6206Device : public CDevice
{
public:
    FT6206Device(CI2CMaster *_I2CMaster, u8 _address = 0x38, u8 _threshold = 128);
    ~FT6206Device();
    boolean Initialize(void);
    // call this about 60 times per second
    void Update(void);
    void RegisterEventHandler(TSC2046EventHandler *_eventHandler);

private:
    boolean WriteRead(const void *_txBuffer, unsigned _txCount,
                      void *_rxBuffer, unsigned _rxCount);

private:
    CI2CMaster *I2CMaster;
    TSC2046EventHandler *eventHandler;
    u8 address;
    unsigned threshold;
    boolean touched;
    unsigned posX;
    unsigned posY;
};

#endif // _ft6206_h
