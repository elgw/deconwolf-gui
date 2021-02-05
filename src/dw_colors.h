#ifndef __dw_colors_h
#define __dw_colors_h
#include <math.h>
#include <stdlib.h>

/* A simple library build to convert wavelengths in nanometers
 * to RGB colors
 * Using `CIE 1964 supplementary standard colorimetric observer`
 * for this purpose and linear transformation from XYZ to RBG.
 *
 * Would be nice to interpolate the CIE table, not just pick the closest
 * row.
 */

typedef struct{
    double R;
    double G;
    double B;
} DwRGB;

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
