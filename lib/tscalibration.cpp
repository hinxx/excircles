//
// tscalibration.cpp
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
#include <circle/logger.h>
#include <assert.h>
#include <excircles/tscalibration.h>

static const char FromTslib[] = "tslib";

TsCalibration::TsCalibration(unsigned _fbWidth, unsigned _fbHeight, unsigned _rotation, boolean _swapXY)
    : fbWidth(_fbWidth),
      fbHeight(_fbHeight),
      swapXY(_swapXY),
      rotation(_rotation),
      // default values that leave ts numbers unchanged after transform
      coeff{0, 1, 0, 0, 0, 1, 1}
{
}

TsCalibration::~TsCalibration(void)
{
}

void TsCalibration::AddCalibrationPoint(unsigned _index, int _fbx, int _fby, int _tsx, int _tsy)
{
    assert(_index < 5);

    points.x[_index] = _tsx;
    points.y[_index] = _tsy;
    points.xfb[_index] = _fbx;
    points.yfb[_index] = _fby;
}

boolean TsCalibration::PerformCalibration(void)
{
    CLogger::Get()->Write(FromTslib, LogDebug, " # |  FBX  FBY  TSX  TSY");
    for (unsigned i = 0; i < 5; i++) {
        CLogger::Get()->Write(FromTslib, LogDebug, "%2d | %4d %4d %4d %4d",
                              i, points.xfb[i], points.yfb[i], points.x[i], points.y[i]);
    }

    // calculate the coefficients

    int j;
	float n, x, y, x2, y2, xy, z, zx, zy;
	float det, a, b, c, e, f, i;
	float scaling = 65536.0;

	// get sums for matrix
	n = x = y = x2 = y2 = xy = 0;
	for (j = 0; j < 5; j++) {
		n += 1.0;
		x += (float)points.x[j];
		y += (float)points.y[j];
		x2 += (float)(points.x[j]*points.x[j]);
		y2 += (float)(points.y[j]*points.y[j]);
		xy += (float)(points.x[j]*points.y[j]);
	}

	// get determinant of matrix -- check if determinant is too small
	det = n*(x2*y2 - xy*xy) + x*(xy*y - x*y2) + y*(x*xy - y*x2);
	if (det < 0.1 && det > -0.1) {
		return FALSE;
	}

	// get elements of inverse matrix
	a = (x2*y2 - xy*xy)/det;
	b = (xy*y - x*y2)/det;
	c = (x*xy - y*x2)/det;
	e = (n*y2 - y*y)/det;
	f = (x*y - n*xy)/det;
	i = (n*x2 - x*x)/det;

	// get sums for x calibration
	z = zx = zy = 0;
	for (j = 0; j < 5; j++) {
		z += (float)points.xfb[j];
		zx += (float)(points.xfb[j]*points.x[j]);
		zy += (float)(points.xfb[j]*points.y[j]);
	}

	// now multiply out to get the calibration for framebuffer x coord
	points.a[0] = (int)((a*z + b*zx + c*zy)*(scaling));
	points.a[1] = (int)((b*z + e*zx + f*zy)*(scaling));
	points.a[2] = (int)((c*z + f*zx + i*zy)*(scaling));

	// get sums for y calibration
	z = zx = zy = 0;
	for (j = 0; j < 5; j++) {
		z += (float)points.yfb[j];
		zx += (float)(points.yfb[j]*points.x[j]);
		zy += (float)(points.yfb[j]*points.y[j]);
	}

	// now multiply out to get the calibration for framebuffer y coord
	points.a[3] = (int)((a*z + b*zx + c*zy)*(scaling));
	points.a[4] = (int)((b*z + e*zx + f*zy)*(scaling));
	points.a[5] = (int)((c*z + f*zx + i*zy)*(scaling));

	// if we got here, we're OK, so assign scaling to a[6] and return
	points.a[6] = (int)scaling;

    // store the coefficients
    SetCalibrationCoefficients(points.a, 7);

    return TRUE;
}

void TsCalibration::SetCalibrationCoefficients(int *_coeff, unsigned _count)
{
    assert(_count == 7);

    for (unsigned i = 0; i < 7; i++) {
        coeff[i] = _coeff[i];
    }
    CLogger::Get()->Write(FromTslib, LogDebug, "Calibration results: %d %d %d %d %d %d %d",
                          coeff[0], coeff[1], coeff[2], coeff[3], coeff[4], coeff[5], coeff[6]);
}

void TsCalibration::ApplyCalibration(int _rawx, int _rawy, int *_x, int *_y)
{
    //CLogger::Get()->Write(FromTslib, LogDebug, "BEFORE CALIB %4d %4d", _rawx, _rawy);

    int x = (coeff[0] + coeff[1] * _rawx + coeff[2] * _rawy) / coeff[6];
    int y = (coeff[3] + coeff[4] * _rawx + coeff[5] * _rawy) / coeff[6];
    int tmp;

    if (swapXY) {
        tmp = x;
        x = y;
        y = tmp;
    }

    switch (rotation) {
    case TsLibRotationNone:
        break;
    case TsLibRotation90:
        tmp = x;
        x = y;
        y = fbWidth - tmp - 1;
        break;
    case TsLibRotation180:
        x = fbWidth - x - 1;
        y = fbHeight - y - 1;
        break;
    case TsLibRotation270:
        tmp = x;
        x = fbHeight - y - 1;
        y = tmp;
        break;
    default:
        CLogger::Get()->Write(FromTslib, LogPanic, "invalid rotation %d", rotation);
        break;
    }

    *_x = x;
    *_y = y;
    //CLogger::Get()->Write(FromTslib, LogDebug, "AFTER CALIB %4d %4d", *_x, *_y);
}
