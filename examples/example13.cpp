/* SISL - SINTEF Spline Library version 4.4.                              */
/* Definition and interrogation of NURBS curves and surface.              */
/* Copyright (C) 1978-2005, SINTEF ICT, Applied Mathematics, Norway.      */

/* This program is free software; you can redistribute it and/or          */
/* modify it under the terms of the GNU General Public License            */
/* as published by the Free Software Foundation version 2 of the License. */

/* This program is distributed in the hope that it will be useful,        */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/* GNU General Public License for more details.                           */

/* You should have received a copy of the GNU General Public License      */
/* along with this program; if not, write to the Free Software            */
/* Foundation, Inc.,                                                      */
/* 59 Temple Place - Suite 330,                                           */
/* Boston, MA  02111-1307, USA.                                           */

/* Contact information: e-mail: tor.dokken@sintef.no                      */
/* SINTEF ICT, Department of Applied Mathematics,                         */
/* P.O. Box 124 Blindern,                                                 */
/* 0314 Oslo, Norway.                                                     */

/* SISL commercial licenses are available for:                            */
/* - Building commercial software.                                        */
/* - Building software whose source code you wish to keep private.        */

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cstdlib>

#include "sisl.h"
#include "GoReadWrite.h"

using namespace std;

namespace {
    string IN_FILE_SURFACE_1 = "example10_surf.g2";
    string IN_FILE_SURFACE_2 = "example11_surf.g2";
    string OUT_FILE_CURVE   = "example13_isectcurves.g2";

    string DESCRIPTION = 
    //==========================================================
    "This program computes the intersection curves between two \n"
    "surfaces.  The two surfaces used have been generated by \n"
    "earlier example programs (example10 and example11), so you\n"
    "should run these first. The filenames for these surfaces \n"
    "are '" + IN_FILE_SURFACE_1 + "' and '" + IN_FILE_SURFACE_2 +
    "'.  \n The resulting curves will be written to the file \n'" 
    + OUT_FILE_CURVE + "'.\n\n";
    //==========================================================

}; // end anonymous namespace 

//===========================================================================
int main(int avnum, char** vararg)
//===========================================================================
{
    cout << '\n' << vararg[0] << ":\n" << DESCRIPTION << endl;
    cout << "To proceed, press enter, or ^C to quit." << endl;
    getchar();

    try {
	ifstream is_sf1(IN_FILE_SURFACE_1.c_str());
	ifstream is_sf2(IN_FILE_SURFACE_2.c_str());
	if (!is_sf1 || !is_sf2) {
	    throw runtime_error("Could not open input files.  Hav you run "
				"the necessary example programs? (example10 "
				"and example11).");
	}

	ofstream os(OUT_FILE_CURVE.c_str());
	if (!os) {
	    throw runtime_error("Unable to open output file.");
	}

	// reading surfaces
	SISLSurf* surf_1 = readGoSurface(is_sf1);
	SISLSurf* surf_2 = readGoSurface(is_sf2);

	// detecting (but not tracing out) intersection curves
	double epsco = 1.0e-15; // computational epsilon
	double epsge = 1.0e-5; // geometric tolerance
	int num_int_points = 0; // number of detected intersection points
	double* intpar_surf_1  = 0; // parameter values for the surface in the intersections
	double* intpar_surf_2 = 0; // parameter values for the curve in the intersections
	int num_int_curves = 0;   // number of intersection curves
	SISLIntcurve** intcurve = 0; // pointer to array of detected intersection curves
	int jstat; // status variable

	// calculating topology of intersections
	s1859(surf_1,          // the first surface
	      surf_2,          // the second surface
	      epsco,           // computational resolution
	      epsge,           // geometry resolution
	      &num_int_points, // number of single intersection points
	      &intpar_surf_1,  // pointer to array of parameter values for surface 1
	      &intpar_surf_2,  //               -"-                    for surface 2
	      &num_int_curves, // number of detected intersection curves
	      &intcurve,       // pointer to array of detected intersection curves.
	      &jstat);         // status variable
	
	if (jstat < 0) {
	    throw runtime_error("Error occured inside call to SISL routine s1859.");
	} else if (jstat > 0) {
	    cerr << "WARNING: warning occured inside call to SISL routine s1859. \n" 
		 << endl;
	}

	// In this example, we expect to detect two intersection curves, but no isolated
	// intersection points.

	cout << "Number of intersection points detected: " << num_int_points << endl;
	cout << "Number of intersection curves detected: " << num_int_curves << endl;
	
	// evaluating (tracing out) intersection curves and writing them to file
	int i;
	for (i = 0; i < num_int_curves; ++i) {
	    s1310(surf_1,          // the first surface
		  surf_2,          // the second surface
		  intcurve[i],     // pointer to the intersection curve object 
		  epsge,           // geometric tolerance
		  double(0),       // maximum step size (ignored if <= 0)
		  1,               // make only 3D curve (no 2D curve in parametric domain)
		  0,               // don't draw the curve
		  &jstat);
	    if (jstat < 0) {
		throw runtime_error("Error occured inside call to SISL routine s1310.");
	    } else if (jstat == 3) {
		throw runtime_error("Iteration stopped due to singular point or degenerate "
				    "surface.");
	    }
	    writeGoCurve(intcurve[i]->pgeom, os);
	}

	// cleaning up
	freeSurf(surf_1);
	freeSurf(surf_2);
	is_sf1.close();
	is_sf2.close();
	os.close();
	free(intpar_surf_1);
	free(intpar_surf_2);
	for (i = 0; i < num_int_curves; ++i) {
	    freeIntcurve(intcurve[i]);
	}
	free(intcurve);

    } catch (exception& e) {
	cerr << "Exception thrown: " << e.what() << endl;
	return 0;
    }

    return 1;
};