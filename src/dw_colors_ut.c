#include "dw_colors.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

void show(double lambda)
{
    DwRGB * C = dw_RGB_new_from_lambda(lambda);
    DwXYZ * C2 = dw_XYZ_new_from_lambda(lambda);
    DwRGB * nC = malloc(sizeof(DwRGB));
    memcpy(nC, C, sizeof(DwRGB));
    dw_RGB_normalize(nC);
    assert(nC->R <= 1);
    assert(nC->G <= 1);
    assert(nC->B <= 1);
    printf("Lambda=%f, XYZ = (%f, %f, %f) RGB = (%f, %f, %f) nRGB = (%f, %f, %f)\n",
           lambda,
           C2->X, C2->Y, C2->Z,
           C->R, C->G, C->B,
           nC->R, nC->G, nC->B);
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

    for(double lambda = 200; lambda < 800; lambda+=17)
    {
        show(lambda);
    }

the_end:
    return 0;
}
