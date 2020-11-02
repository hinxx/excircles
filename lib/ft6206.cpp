//
// ft6206.cpp
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
#include <circle/devicenameservice.h>
#include <circle/logger.h>
#include <assert.h>

#include <excircles/ft6206.h>

// register addresses
#define FT6206_REG_NUMTOUCHES       0x02
#define FT6206_REG_P1_XH            0x03
#define FT6206_REG_P1_XL            0x04
#define FT6206_REG_P1_YH            0x05
#define FT6206_REG_P1_YL            0x06
#define FT6206_REG_THRESHOLD        0x80
#define FT6206_REG_CHIP_ID          0xA3
#define FT6206_REG_VENDOR_ID        0xA8
// supported device IDs
#define FT6206_VENDOR_ID            0x11
#define FT6206_CHIP_ID              0x06

#define FT6206_TOUCH_DOWN           0
#define FT6206_TOUCH_UP             1
#define FT6206_TOUCH_CONTACT        2

static const char FromFT6206[] = "ft6206";

FT6206Device::FT6206Device(CI2CMaster *_I2CMaster, u8 _address, u8 _threshold)
    : I2CMaster(_I2CMaster),
      eventHandler(0),
      address(_address),
      threshold(_threshold),
      touched(FALSE)
{
    assert(I2CMaster != 0);
    assert(address > 0);
}

FT6206Device::~FT6206Device()
{
	I2CMaster = 0;
}

boolean FT6206Device::Initialize(void)
{
    assert(I2CMaster != 0);

    u8 txBuffer[2] = { 0 };
    u8 rxData = 0;

    txBuffer[0] = FT6206_REG_VENDOR_ID;
    if (! WriteRead(txBuffer, 1, &rxData, 1)) {
        return FALSE;
    }
    u8 vendorID = rxData;

    txBuffer[0] = FT6206_REG_CHIP_ID;
    if (! WriteRead(txBuffer, 1, &rxData, 1)) {
        return FALSE;
    }
    u8 chipID = rxData;

    txBuffer[0] = FT6206_REG_THRESHOLD;
    txBuffer[1] = threshold;
    if (! WriteRead(txBuffer, 2, 0, 0)) {
        return FALSE;
    }

    if (vendorID == FT6206_VENDOR_ID && chipID == FT6206_CHIP_ID) {
        CLogger::Get()->Write(FromFT6206, LogNotice, "Detected FT6206 device");
    } else {
        CLogger::Get()->Write(FromFT6206, LogWarning, "Unsupported device 0x%02X%02X",
                              vendorID, chipID);
        return FALSE;
    }

    CDeviceNameService::Get()->AddDevice("touch1", this, FALSE);

    return TRUE;
}

void FT6206Device::Update(void)
{
    assert(I2CMaster != 0);

    // select first register
    u8 txBuffer = 0x00;
    // read 16 registers
    u8 rxData[16] = { 0 };
    if (! WriteRead(&txBuffer, 1, rxData, sizeof(rxData))) {
        return;
    }

    unsigned x = ((rxData[FT6206_REG_P1_XH] & 0x0F) << 8) | rxData[FT6206_REG_P1_XL];
    unsigned y = ((rxData[FT6206_REG_P1_YH] & 0x0F) << 8) | rxData[FT6206_REG_P1_YL];
    //unsigned eventID = (rxData[FT6206_REG_P1_XH] & 0xC0) >> 6;
    //unsigned touchID = rxData[FT6206_REG_P1_YH] >> 4;

    // nothing to do if we no pressure detected or no release event to report
    if (! (rxData[FT6206_REG_NUMTOUCHES] & 0x01) && (! touched)) {
        return;
    }

    if (rxData[FT6206_REG_NUMTOUCHES] & 0x01) {
        if (! touched) {
            posX = x;
            posY = y;

            if (eventHandler != 0) {
                (*eventHandler)(FT6206EventFingerDown, 0, x, y);
            }
        } else {
            if (x != posX || y != posY) {
                posX = x;
                posY = y;

                if (eventHandler != 0) {
                    (*eventHandler)(FT6206EventFingerMove, 0, x, y);
                }
            }
        }
        touched = TRUE;
    } else {
        if (eventHandler != 0) {
            (*eventHandler)(FT6206EventFingerUp, 0, 0, 0);
        }
        touched = FALSE;
    }
}

void FT6206Device::RegisterEventHandler(TSC2046EventHandler *_eventHandler)
{
    assert(eventHandler == 0);
    eventHandler = _eventHandler;
    assert(eventHandler != 0);
}

boolean FT6206Device::WriteRead(const void *_txBuffer, unsigned _txCount,
                                 void *_rxBuffer, unsigned _rxCount)
{
    assert(_txBuffer != 0);

    if (I2CMaster->Write(address, _txBuffer, _txCount) != (int) _txCount) {
        CLogger::Get()->Write(FromFT6206, LogError, "I2C write error");
        return FALSE;
    }
    if (_rxBuffer) {
        if (I2CMaster->Read(address, _rxBuffer, _rxCount) != (int) _rxCount) {
            CLogger::Get()->Write(FromFT6206, LogError, "I2C read error");
            return FALSE;
        }
    }

    return TRUE;
}
