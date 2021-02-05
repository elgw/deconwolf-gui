#ifndef __dw_colors_h
#define __dw_colors_h
#include <math.h>
#include <stdlib.h>

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
