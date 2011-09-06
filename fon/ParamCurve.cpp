/* ParamCurve.cpp
 *
 * Copyright (C) 1992-2011 Paul Boersma
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "ParamCurve.h"

#include "oo_DESTROY.h"
#include "ParamCurve_def.h"
#include "oo_COPY.h"
#include "ParamCurve_def.h"
#include "oo_EQUAL.h"
#include "ParamCurve_def.h"
#include "oo_DESCRIPTION.h"
#include "ParamCurve_def.h"
#include "oo_CAN_WRITE_AS_ENCODING.h"
#include "ParamCurve_def.h"

Thing_implement (ParamCurve, Function, 2);

void structParamCurve :: v_info () {
	double xmin = 1e300, xmax = -1e300, ymin = 1e300, ymax = -1e300;
	for (long i = 1; i <= x -> nx; i ++) {
		double value = x -> z [1] [i];
		if (value < xmin) xmin = value;
		if (value > xmax) xmax = value;
	}
	for (long i = 1; i <= y -> nx; i ++) {
		double value = y -> z [1] [i];
		if (value < ymin) ymin = value;
		if (value > ymax) ymax = value;
	}
	structData :: v_info ();
	MelderInfo_writeLine1 (L"Domain:");
	MelderInfo_writeLine2 (L"   tmin: ", Melder_double (xmin));
	MelderInfo_writeLine2 (L"   tmax: ", Melder_double (xmax));
	MelderInfo_writeLine1 (L"x sampling:");
	MelderInfo_writeLine2 (L"   Number of values of t in x: ", Melder_double (x -> nx));
	MelderInfo_writeLine5 (L"   t step in x: ", Melder_double (x -> dx), L" (sampling rate ", Melder_double (1.0 / x -> dx), L")");
	MelderInfo_writeLine2 (L"   First t in x: ", Melder_double (x -> x1));
	MelderInfo_writeLine1 (L"x values:");
	MelderInfo_writeLine2 (L"   Minimum x: ", Melder_double (xmin));
	MelderInfo_writeLine2 (L"   Maximum x: ", Melder_double (xmax));
	MelderInfo_writeLine1 (L"y sampling:");
	MelderInfo_writeLine2 (L"   Number of values of t in y: ", Melder_double (y -> nx));
	MelderInfo_writeLine5 (L"   t step in y: ", Melder_double (y -> dx), L" (sampling rate ", Melder_double (1.0 / y -> dx), L")");
	MelderInfo_writeLine2 (L"   First t in y: ", Melder_double (y -> x1));
	MelderInfo_writeLine1 (L"y values:");
	MelderInfo_writeLine2 (L"   Minimum y: ", Melder_double (ymin));
	MelderInfo_writeLine2 (L"   Maximum y: ", Melder_double (ymax));
}

void structParamCurve :: v_writeText (MelderFile file) {
	Data_writeText (x, file);
	Data_writeText (y, file);
}

void structParamCurve :: v_readText (MelderReadText text) {
	x = Thing_new (Sound); therror
	y = Thing_new (Sound); therror
	Data_readText (x, text); therror
	Data_readText (y, text); therror
	xmin = x -> xmin > y -> xmin ? x -> xmin : y -> xmin;
	xmax = x -> xmax < y -> xmax ? x -> xmax : y -> xmax;
}

void structParamCurve :: v_writeBinary (FILE *f) {
	Data_writeBinary (x, f); therror
	Data_writeBinary (y, f); therror
}

void structParamCurve :: v_readBinary (FILE *f) {
	long saveVersion = Thing_version;
	Thing_version = 2;
	x = Thing_new (Sound); therror
	y = Thing_new (Sound); therror
	Data_readBinary (x, f); therror
	Data_readBinary (y, f); therror
	Thing_version = saveVersion;
	xmin = x -> xmin > y -> xmin ? x -> xmin : y -> xmin;
	xmax = x -> xmax < y -> xmax ? x -> xmax : y -> xmax; 
}

void ParamCurve_init (I, Any void_x, Any void_y) {
	iam (ParamCurve);
	Sound x = (Sound) void_x, y = (Sound) void_y;
	if (x -> xmax <= y -> xmin || x -> xmin >= y -> xmax)
		Melder_throw ("Domains do not overlap.");
	my x = Data_copy (x);
	my y = Data_copy (y);
	my xmin = x -> xmin > y -> xmin ? x -> xmin : y -> xmin;
	my xmax = x -> xmax < y -> xmax ? x -> xmax : y -> xmax; 
}

ParamCurve ParamCurve_create (Any x, Any y) {
	try {
		autoParamCurve me = Thing_new (ParamCurve);
		ParamCurve_init (me.peek(), x, y); therror
		return me.transfer();
	} catch (MelderError) {
		Melder_throw ("ParamCurve not created.");
	}
}

void ParamCurve_draw (I, Graphics g, double t1, double t2, double dt,
	double x1, double x2, double y1, double y2, int garnish)
{
	iam (ParamCurve);
	if (t2 <= t1) {
		double tx1 = my x -> x1;
		double ty1 = my y -> x1;
		double tx2 = my x -> x1 + (my x -> nx - 1) * my x -> dx;
		double ty2 = my y -> x1 + (my y -> nx - 1) * my y -> dx;
		t1 = tx1 > ty1 ? tx1 : ty1;
		t2 = tx2 < ty2 ? tx2 : ty2;
	}
	if (x2 <= x1) Matrix_getWindowExtrema (my x, 0, 0, 1, 1, & x1, & x2);
	if (x1 == x2) { x1 -= 1.0; x2 += 1.0; }
	if (y2 <= y1) Matrix_getWindowExtrema (my y, 0, 0, 1, 1, & y1, & y2);
	if (y1 == y2) { y1 -= 1.0; y2 += 1.0; }
	if (dt <= 0.0)
		dt = my x -> dx < my y -> dx ? my x -> dx : my y -> dx;
	long numberOfPoints = (long) ceil ((t2 - t1) / dt) + 1;
	if (numberOfPoints > 0) {
		autoNUMvector <double> x (1, numberOfPoints);
		autoNUMvector <double> y (1, numberOfPoints);
		for (long i = 1; i <= numberOfPoints; i ++) {
			double t = i == numberOfPoints ? t2 : t1 + (i - 1) * dt;
			double index = Sampled_xToIndex (my x, t);
			x [i] = NUM_interpolate_sinc (my x -> z [1], my x -> nx, index, 50);
			index = Sampled_xToIndex (my y, t);
			y [i] = NUM_interpolate_sinc (my y -> z [1], my y -> nx, index, 50);
		}
		Graphics_setWindow (g, x1, x2, y1, y2);
		Graphics_setInner (g);
		Graphics_polyline (g, numberOfPoints, & x [1], & y [1]);
		Graphics_unsetInner (g);
	}
	if (garnish) {
		Graphics_drawInnerBox (g);
		Graphics_marksBottom (g, 2, 1, 1, 0);
		Graphics_marksLeft (g, 2, 1, 1, 0);
	}
}

void ParamCurve_swapXY (I) {
	iam (ParamCurve);
	Sound help = my x;
	my x = my y;
	my y = help;
}

/* End of file ParamCurve.cpp */