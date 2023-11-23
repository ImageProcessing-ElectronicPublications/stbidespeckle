/*
https://github.com/heptagonhust/bicubic-image-resize/issues/9
*/

#ifndef DESPECKLE_H_
#define DESPECKLE_H_

#include <stdbool.h>
#include <math.h>
#define DESPECKLE_VERSION "1.0"

#ifdef DESPECKLE_STATIC
#define DESPECKLEAPI static
#else
#define DESPECKLEAPI extern
#endif

#ifdef __cplusplus
extern "C" {
#endif
DESPECKLEAPI void Despeckle (unsigned char *src, int height, int width, int channels, int radius, float coef, unsigned char *res);
#ifdef __cplusplus
}
#endif

#ifdef DESPECKLE_IMPLEMENTATION

typedef struct
{
    unsigned char c[4];
}
pixel;

typedef struct
{
    float c[4];
}
pixelf;

static pixel PixelCopy(pixel p, int channels)
{
    int d;
    pixel r;

    for (d = 0; d < channels; d++)
    {
        r.c[d] = p.c[d];
    }

    return r;
}

static float PixelDist (pixel pA, pixel pB, int channels)
{
    int d;
    float dist = 0.0f;

    for(d = 0; d < channels; d++)
    {
        dist += (pB.c[d] - pA.c[d]) * (pB.c[d] - pA.c[d]);
    }

    return dist;
}

static float PixelCross (pixel pA, pixel pB,pixel pC, pixel pD, int channels)
{
    int d;
    float cross = 0.0f;

    for(d = 0; d < channels; d++)
    {
        cross += (pB.c[d] - pA.c[d]) * (pD.c[d] - pC.c[d]);
    }

    return cross;
}

static pixel PixelMean9 (pixel pA, pixel pB,pixel pC, pixel pD, pixel pE, pixel pF, pixel pG, pixel pH, pixel pI, int channels)
{
    int d, sum;
    pixel mean;

    for(d = 0; d < channels; d++)
    {
        sum = pA.c[d] + pB.c[d] + pC.c[d] + pD.c[d] + pE.c[d] + pF.c[d] + pG.c[d] + pH.c[d] + pI.c[d];
        sum += 4;
        sum /= 9;
        mean.c[d] = sum;
    }

    return mean;
}

static pixelf PixelGrad (pixel p1, pixel p2, pixel p3, pixel p4, int channels)
{
    int d;
    pixelf pR;

    for(d = 0; d < channels; d++)
    {
        pR.c[d] = p1.c[d];
        pR.c[d] += p2.c[d];
        pR.c[d] -= p3.c[d];
        pR.c[d] -= p4.c[d];
    }

    return pR;
}

static pixelf PixelDelta (pixelf px, pixelf py, pixelf pz, float dx, float dy, float dz, int channels)
{
    int d;
    pixelf pR;

    for(d = 0; d < channels; d++)
    {
        pR.c[d] = (py.c[d] * dx + px.c[d] * dy - 0.5f * pz.c[d] * dz);
    }

    return pR;
}

static pixel PixelAlign (pixel p0, pixel pM, pixelf pd, float coef, int channels)
{
    int d;
    float sum;
    pixel pR;

    for(d = 0; d < channels; d++)
    {
        sum = p0.c[d];
        if ((pM.c[d] > 128 && pd.c[d] > 0.0f) || (pM.c[d] < 128 && pd.c[d] < 0.0f))
        {
            sum += (coef * pd.c[d]);
            sum += 0.5f;
            sum = (sum < 0.0f) ? 0.0f : (sum < 255.0f) ? sum : 255.0f;
        }
        pR.c[d] = (int) sum;
    }

    return pR;
}

static pixel PixelGet (unsigned char *image, int height, int width, int channels, int y, int x)
{
    int d;
    pixel p;
    size_t k;

    y = (y < 0) ? 0 : (y < height) ? y : (height - 1);
    x = (x < 0) ? 0 : (x < width) ? x : (width - 1);
    k = ((width * y) + x) * channels;

    for (d = 0; d < channels; d++)
    {
        p.c[d] = image[k + d];
    }

    return p;
}

static void PixelSet (unsigned char *image, int height, int width, int channels, int y, int x, pixel p)
{
    int d;
    size_t k;

    y = (y < 0) ? 0 : (y < height) ? y : (height - 1);
    x = (x < 0) ? 0 : (x < width) ? x : (width - 1);
    k = ((width * y) + x) * channels;

    for (d = 0; d < channels; d++)
    {
        image[k + d] = p.c[d];
    }
}

DESPECKLEAPI void Despeckle (unsigned char *src, int height, int width, int channels, int radius, float coef, unsigned char *res)
{
    int j, y, x;
    pixel pA, pB, pC, pD, pE, pF, pG, pH, pI, pM, pR;
    float dIy, dIx, dIxy, dI;
    pixelf gy, gx, gxy, gd2;

    for (j = 0; j < radius; j++)
    {
        for (y = 0; y < height; y++)
        {
            for (x = 0; x < width; x++)
            {
                pA = PixelGet(src, height, width, channels, y - 1, x - 1);
                pB = PixelGet(src, height, width, channels, y - 1, x);
                pC = PixelGet(src, height, width, channels, y - 1, x + 1);
                pD = PixelGet(src, height, width, channels, y, x - 1);
                pE = PixelGet(src, height, width, channels, y, x);
                pF = PixelGet(src, height, width, channels, y, x + 1);
                pG = PixelGet(src, height, width, channels, y + 1, x - 1);
                pH = PixelGet(src, height, width, channels, y + 1, x);
                pI = PixelGet(src, height, width, channels, y + 1, x + 1);
                dIy = PixelDist(pH, pB, channels);
                dIx = PixelDist(pF, pD, channels);
                dIxy = PixelCross(pB, pH, pD, pF, channels);
                dI = dIy + dIx;
                pM = PixelMean9(pA, pB, pC, pD, pE, pF, pG, pH, pI, channels);

                if (dI > 0.0f)
                {
                    dIx /= dI;
                    dIy /= dI;
                    dIxy /= dI;
                    gy = PixelGrad(pB, pH, pE, pE, channels);
                    gx = PixelGrad(pD, pF, pE, pE, channels);
                    gxy = PixelGrad(pC, pG, pA, pI, channels);
                    gd2 = PixelDelta(gy, gx, gxy, dIx, dIy, dIxy, channels);
                    pR = PixelAlign(pE, pM, gd2, coef, channels);
                }
                else
                {
                    pR = pM;
                }                

                PixelSet(res, height, width, channels, y, x, pR);
            }
        }
        for (y = 0; y < height; y++)
        {
            for (x = 0; x < width; x++)
            {
                pR = PixelGet(res, height, width, channels, y, x);
                PixelSet(src, height, width, channels, y, x, pR);
            }
        }
    }
}

#endif /* DESPECKLE_IMPLEMENTATION */

#endif /* DESPECKLE_H_ */
