//
// tsc2046.cpp
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
#include <circle/devicenameservice.h>
#include <circle/logger.h>
#include <assert.h>

#include <excircles/tsc2046.h>

static const char FromTSC2046[] = "tsc2046";

#ifndef USE_SPI_MASTER_AUX
TSC2046Device::TSC2046Device(CSPIMaster *_SPIMaster, unsigned _cs, unsigned _threshold)
#else
TSC2046Device::TSC2046Device(CSPIMasterAUX *_SPIMaster, unsigned _cs, unsigned _threshold)
#endif
    : SPIMaster(_SPIMaster) ,
      eventHandler(0),
      cs(_cs),
      threshold(_threshold),
      touched(FALSE)
{
}

TSC2046Device::~TSC2046Device(void)
{
    SPIMaster = 0;
}

boolean TSC2046Device::Initialize(void)
{
    // perform a dummy read to see if the device responds
    u8 txBuffer[3] = {0};
    u8 rxBuffer[3] = {0};
    txBuffer[0] = 0xB0;
    if (SPIMaster->WriteRead(cs, txBuffer, rxBuffer, sizeof(txBuffer)) != sizeof(txBuffer)) {
        CLogger::Get()->Write(FromTSC2046, LogError, "SPI write/read error");
        return FALSE;
    }

    CDeviceNameService::Get()->AddDevice("touch1", this, FALSE);

    return TRUE;
}

void TSC2046Device::Update(void)
{
    assert(SPIMaster != 0);

    u8 txBuffer[36] = {0};
    u8 rxBuffer[36] = {0};

    // Z1
    txBuffer[0] = 0xB1;
    txBuffer[3] = 0xB1;
    txBuffer[6] = 0xB1;

    // Z2
    txBuffer[9] = 0xC1;
    txBuffer[12] = 0xC1;
    txBuffer[15] = 0xC1;

    // X
    txBuffer[18] = 0x91;
    txBuffer[21] = 0x91;
    txBuffer[24] = 0x91;

    // Y
    txBuffer[27] = 0xD1;
    txBuffer[30] = 0xD1;
    txBuffer[33] = 0xD0;

    if (SPIMaster->WriteRead(cs, txBuffer, rxBuffer, sizeof(txBuffer)) != sizeof(txBuffer)) {
        CLogger::Get()->Write(FromTSC2046, LogError, "SPI write/read error");
        return;
    }

    unsigned z1 = (rxBuffer[7] << 8 | rxBuffer[8]) >> 3;
    unsigned z2 = (rxBuffer[16] << 8 | rxBuffer[17]) >> 3;
    unsigned x = (rxBuffer[25] << 8 | rxBuffer[26]) >> 3;
    unsigned y = (rxBuffer[34] << 8 | rxBuffer[35]) >> 3;
    unsigned z = z1 + 4095;
    z -= z2;

    // nothing to do if we no pressure detected or no release event to report
    if ((z < threshold) && (! touched)) {
        return;
    }

    if (z > threshold) {
        if (! touched) {
            posX = x;
            posY = y;

            if (eventHandler != 0) {
                (*eventHandler)(TSC2046EventFingerDown, 0, x, y);
            }
        } else {
            if (x != posX || y != posY) {
                posX = x;
                posY = y;

                if (eventHandler != 0) {
                    (*eventHandler)(TSC2046EventFingerMove, 0, x, y);
                }
            }
        }
        touched = TRUE;
    } else {
        if (eventHandler != 0) {
            (*eventHandler)(TSC2046EventFingerUp, 0, 0, 0);
        }
        touched = FALSE;
    }
}

void TSC2046Device::RegisterEventHandler(TSC2046EventHandler *_eventHandler)
{
    assert(eventHandler == 0);
    eventHandler = _eventHandler;
    assert(eventHandler != 0);
}
