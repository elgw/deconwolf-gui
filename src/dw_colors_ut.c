#include "dw_colors.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

double double_max(double a, double b)
{
    if(a > b)
    {
        return a;
    }
    return b;
}


void show(double lambda)
{
    DwRGB * C = dw_RGB_new_from_lambda(lambda);
    DwXYZ * C2 = dw_XYZ_new_from_lambda(lambda);


    assert(C->R <= 1);
    assert(C->G <= 1);
    assert(C->B <= 1);
    double scaling = double_max(C->R, double_max(C->G, C->B));
    printf("Lambda=%f, XYZ = (%f, %f, %f) RGB = (%f, %f, %f) nRGB = (%f, %f, %f)\n",
           lambda,
           C2->X, C2->Y, C2->Z,
           C->R, C->G, C->B,
           C->R/scaling, C->G/scaling, C->B/scaling);
    printf("RGB = #%02X%02X%02X\n",
           (int) round(255.0*C->R),
           (int) round(255.0*C->G),
           (int) round(255.0*C->B));
    printf("\e[38;2;%d;%d;%dm\e[48;2;%d;%d;%dm Approx color in background \e[0m\n",
           (int) round(255.0 - 255.0*C->R),
           (int) round(255.0 - 255.0*C->G),
           (int) round(255.0 - 255.0*C->B),
           (int) round(255.0*C->R),
           (int) round(255.0*C->G),
           (int) round(255.0*C->B));
    free(C2);
    free(C);
    return;
}


void show_matlab(double lambda)
{
    DwRGB * C = dw_RGB_new_from_lambda(lambda);
    DwXYZ * C2 = dw_XYZ_new_from_lambda(lambda);

    assert(C->R <= 1);
    assert(C->G <= 1);
    assert(C->B <= 1);
    double scaling = double_max(C->R, double_max(C->G, C->B));
    scaling <= 0 ? scaling = 1 : 0;
    printf("[%f, %f, %f, %f, %f, %f, %f, %f, %f, %f]",
           lambda,
           C2->X, C2->Y, C2->Z,
           C->R, C->G, C->B,
           C->R/scaling, C->G/scaling, C->B/scaling);
    free(C2);
    free(C);

    return;
}


int main(int argc, char ** argv)
{
    if(argc == 2)
    {
        double lambda = atof(argv[1]);
        show(lambda);
        goto the_end;
    }

    printf("%% lambda, X, Y, Z, R, G, B, nR, nG, nB\n");
    printf("T = [ ...\n");
    double first_lambda = 360;
    for(double lambda = first_lambda; lambda < 830; lambda+=5)
    {
        if(lambda != first_lambda)
        {
            printf("\n");
        }
        show_matlab(lambda);
    }
    printf("];\n");

the_end:
    return 0;
}
