//
// tscalibration.h
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
#ifndef _tslib_h
#define _tslib_h

#include <circle/types.h>

enum {
    TsLibRotationNone = 0,
    TsLibRotation90,
    TsLibRotation180,
    TsLibRotation270
};

class TsCalibration
{
public:
    TsCalibration(unsigned _fbWidth, unsigned _fbHeight, unsigned _rotation, boolean _swapXY);
    ~TsCalibration(void);

    void AddCalibrationPoint(unsigned _index, int _fbx, int _fby, int _tsx, int _tsy);
    boolean PerformCalibration(void);
    void ApplyCalibration(int _rawx, int _rawy, int *_x, int *_y);
    void SetCalibrationCoefficients(int *_coeff, unsigned _count);

private:

    // screen resolution at the time when calibration was performed
    unsigned fbWidth;
    unsigned fbHeight;

    // x / y axis swapping
    boolean	swapXY;

    // screen rotation
    unsigned rotation;

    // linear scaling and offset parameters for x,y (can include rotation)
    int	coeff[7];

    // calibration points
    struct {
        int x[5];
        int xfb[5];
        int y[5];
        int yfb[5];
    	int a[7];
    } points;
};

#endif // _tslib_h
