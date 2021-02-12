#ifndef __dw_colors_h
#define __dw_colors_h

/**
 * A simple library build to convert wavelengths in nanometers
 * to sRGB colors
 * It uses the `CIE 1964 supplementary standard colorimetric observer`
 * for this purpose, and then a linear transformation from XYZ to RBG.
 *
 * TODO:
 * Would be nice to interpolate the CIE table, not just pick the closest
 * row.
 */

#include <math.h>
#include <stdlib.h>

/** @struct DwRGB
 * A struct to represent an RGB triple.
*/
typedef struct{
    double R;
    double G;
    double B;
} DwRGB;

/** @struct DwXYZ
 * For representation of
*/
typedef struct{
    double X;
    double Y;
    double Z;
} DwXYZ;

DwXYZ * dw_XYZ_new();
DwRGB * dw_RGB_new_from_lambda(double lambda);
DwRGB * dw_RGB_new_from_dw_XYZ(DwXYZ * C);
DwXYZ * dw_XYZ_new_from_lambda(double lambda);

#endif
