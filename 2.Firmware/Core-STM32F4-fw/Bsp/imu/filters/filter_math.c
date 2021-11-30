#include <stdint.h>
#include <math.h>

#include "filter_math.h"

#if defined(FAST_MATH) || defined(VERY_FAST_MATH)
#if defined(VERY_FAST_MATH)

// http://lolengine.net/blog/2011/12/21/better-function-approximations
// Chebyshev http://stackoverflow.com/questions/345085/how-do-trigonometric-functions-work/345117#345117
// Thanks for ledvinap for making such accuracy possible! See: https://github.com/cleanflight/cleanflight/issues/940#issuecomment-110323384
// https://github.com/Crashpilot1000/HarakiriWebstore1/blob/master/src/mw.c#L1235
// sin_approx maximum absolute error = 2.305023e-06
// cos_approx maximum absolute error = 2.857298e-06
#define sinPolyCoef3 -1.666568107e-1f
#define sinPolyCoef5  8.312366210e-3f
#define sinPolyCoef7 -1.849218155e-4f
#define sinPolyCoef9  0
#else
#define sinPolyCoef3 -1.666665710e-1f                                          // Double: -1.666665709650470145824129400050267289858e-1
#define sinPolyCoef5  8.333017292e-3f                                          // Double:  8.333017291562218127986291618761571373087e-3
#define sinPolyCoef7 -1.980661520e-4f                                          // Double: -1.980661520135080504411629636078917643846e-4
#define sinPolyCoef9  2.600054768e-6f                                          // Double:  2.600054767890361277123254766503271638682e-6
#endif

float sin_approx(float x)
{
    int32_t xint = x;
    if (xint < -32 || xint > 32) return 0.0f;                               // Stop here on error input (5 * 360 Deg)
    while (x > M_PIf) x -= (2.0f * M_PIf);                                 // always wrap input angle to -PI..PI
    while (x < -M_PIf) x += (2.0f * M_PIf);
    if (x > (0.5f * M_PIf)) x = (0.5f * M_PIf) - (x - (0.5f * M_PIf));   // We just pick -90..+90 Degree
    else if (x < -(0.5f * M_PIf)) x = -(0.5f * M_PIf) - ((0.5f * M_PIf) + x);
    float x2 = x * x;
    return x + x * x2 * (sinPolyCoef3 + x2 * (sinPolyCoef5 + x2 * (sinPolyCoef7 + x2 * sinPolyCoef9)));
}

float cos_approx(float x)
{
    return sin_approx(x + (0.5f * M_PIf));
}

// Initial implementation by Crashpilot1000 (https://github.com/Crashpilot1000/HarakiriWebstore1/blob/396715f73c6fcf859e0db0f34e12fe44bace6483/src/mw.c#L1292)
// Polynomial coefficients by Andor (http://www.dsprelated.com/showthread/comp.dsp/21872-1.php) optimized by Ledvinap to save one multiplication
// Max absolute error 0,000027 degree
// atan2_approx maximum absolute error = 7.152557e-07 rads (4.098114e-05 degree)
float atan2_approx(float y, float x)
{
#define atanPolyCoef1  3.14551665884836e-07f
#define atanPolyCoef2  0.99997356613987f
#define atanPolyCoef3  0.14744007058297684f
#define atanPolyCoef4  0.3099814292351353f
#define atanPolyCoef5  0.05030176425872175f
#define atanPolyCoef6  0.1471039133652469f
#define atanPolyCoef7  0.6444640676891548f

    float res, absX, absY;
    absX = fabsf(x);
    absY = fabsf(y);
    res = MAX(absX, absY);
    if (res) res = MIN(absX, absY) / res;
    else res = 0.0f;
    res =
        -((((atanPolyCoef5 * res - atanPolyCoef4) * res - atanPolyCoef3) * res - atanPolyCoef2) * res - atanPolyCoef1) /
        ((atanPolyCoef7 * res + atanPolyCoef6) * res + 1.0f);
    if (absY > absX) res = (M_PIf / 2.0f) - res;
    if (x < 0) res = M_PIf - res;
    if (y < 0) res = -res;
    return res;
}

// http://http.developer.nvidia.com/Cg/acos.html
// Handbook of Mathematical Functions
// M. Abramowitz and I.A. Stegun, Ed.
// acos_approx maximum absolute error = 6.760856e-05 rads (3.873685e-03 degree)
float acos_approx(float x)
{
    float xa = fabsf(x);
    float result = sqrtf(1.0f - xa) * (1.5707288f + xa * (-0.2121144f + xa * (0.0742610f + (-0.0187293f * xa))));
    if (x < 0.0f)
        return M_PIf - result;
    else
        return result;
}

#endif

int gcd(int num, int denom)
{
    if (denom == 0)
    {
        return num;
    }

    return gcd(denom, num % denom);
}

int32_t applyDeadband(const int32_t value, const int32_t deadband)
{
    if (ABS(value) < deadband)
    {
        return 0;
    }

    return value >= 0 ? value - deadband : value + deadband;
}

float fapplyDeadband(const float value, const float deadband)
{
    if (fabsf(value) < deadband)
    {
        return 0;
    }

    return value >= 0 ? value - deadband : value + deadband;
}

void devClear(stdev_t *dev)
{
    dev->m_n = 0;
}

void devPush(stdev_t *dev, float x)
{
    dev->m_n++;
    if (dev->m_n == 1)
    {
        dev->m_oldM = dev->m_newM = x;
        dev->m_oldS = 0.0f;
    } else
    {
        dev->m_newM = dev->m_oldM + (x - dev->m_oldM) / dev->m_n;
        dev->m_newS = dev->m_oldS + (x - dev->m_oldM) * (x - dev->m_newM);
        dev->m_oldM = dev->m_newM;
        dev->m_oldS = dev->m_newS;
    }
}

float devVariance(stdev_t *dev)
{
    return ((dev->m_n > 1) ? dev->m_newS / (dev->m_n - 1) : 0.0f);
}

float devStandardDeviation(stdev_t *dev)
{
    return sqrtf(devVariance(dev));
}

float degreesToRadians(int16_t degrees)
{
    return degrees * RAD;
}

int scaleRange(int x, int srcFrom, int srcTo, int destFrom, int destTo)
{
    long int a = ((long int) destTo - (long int) destFrom) * ((long int) x - (long int) srcFrom);
    long int b = (long int) srcTo - (long int) srcFrom;
    return (a / b) + destFrom;
}

float scaleRangef(float x, float srcFrom, float srcTo, float destFrom, float destTo)
{
    float a = (destTo - destFrom) * (x - srcFrom);
    float b = srcTo - srcFrom;
    return (a / b) + destFrom;
}

// Quick median filter implementation
// (c) N. Devillard - 1998
// http://ndevilla.free.fr/median/median.pdf
#define QMF_SORT(a, b) { if ((a)>(b)) QMF_SWAP((a),(b)); }
#define QMF_SWAP(a, b) { int32_t temp=(a);(a)=(b);(b)=temp; }
#define QMF_COPY(p, v, n) { int32_t i; for (i=0; i<n; i++) p[i]=v[i]; }
#define QMF_SORTF(a, b) { if ((a)>(b)) QMF_SWAPF((a),(b)); }
#define QMF_SWAPF(a, b) { float temp=(a);(a)=(b);(b)=temp; }

int32_t quickMedianFilter3(int32_t *v)
{
    int32_t p[3];
    QMF_COPY(p, v, 3);

    QMF_SORT(p[0], p[1]);
    QMF_SORT(p[1], p[2]);
    QMF_SORT(p[0], p[1]);
    return p[1];
}

int32_t quickMedianFilter5(int32_t *v)
{
    int32_t p[5];
    QMF_COPY(p, v, 5);

    QMF_SORT(p[0], p[1]);
    QMF_SORT(p[3], p[4]);
    QMF_SORT(p[0], p[3]);
    QMF_SORT(p[1], p[4]);
    QMF_SORT(p[1], p[2]);
    QMF_SORT(p[2], p[3]);
    QMF_SORT(p[1], p[2]);
    return p[2];
}

int32_t quickMedianFilter7(int32_t *v)
{
    int32_t p[7];
    QMF_COPY(p, v, 7);

    QMF_SORT(p[0], p[5]);
    QMF_SORT(p[0], p[3]);
    QMF_SORT(p[1], p[6]);
    QMF_SORT(p[2], p[4]);
    QMF_SORT(p[0], p[1]);
    QMF_SORT(p[3], p[5]);
    QMF_SORT(p[2], p[6]);
    QMF_SORT(p[2], p[3]);
    QMF_SORT(p[3], p[6]);
    QMF_SORT(p[4], p[5]);
    QMF_SORT(p[1], p[4]);
    QMF_SORT(p[1], p[3]);
    QMF_SORT(p[3], p[4]);
    return p[3];
}

int32_t quickMedianFilter9(int32_t *v)
{
    int32_t p[9];
    QMF_COPY(p, v, 9);

    QMF_SORT(p[1], p[2]);
    QMF_SORT(p[4], p[5]);
    QMF_SORT(p[7], p[8]);
    QMF_SORT(p[0], p[1]);
    QMF_SORT(p[3], p[4]);
    QMF_SORT(p[6], p[7]);
    QMF_SORT(p[1], p[2]);
    QMF_SORT(p[4], p[5]);
    QMF_SORT(p[7], p[8]);
    QMF_SORT(p[0], p[3]);
    QMF_SORT(p[5], p[8]);
    QMF_SORT(p[4], p[7]);
    QMF_SORT(p[3], p[6]);
    QMF_SORT(p[1], p[4]);
    QMF_SORT(p[2], p[5]);
    QMF_SORT(p[4], p[7]);
    QMF_SORT(p[4], p[2]);
    QMF_SORT(p[6], p[4]);
    QMF_SORT(p[4], p[2]);
    return p[4];
}

float quickMedianFilter3f(float *v)
{
    float p[3];
    QMF_COPY(p, v, 3);

    QMF_SORTF(p[0], p[1]);
    QMF_SORTF(p[1], p[2]);
    QMF_SORTF(p[0], p[1]);
    return p[1];
}

float quickMedianFilter5f(float *v)
{
    float p[5];
    QMF_COPY(p, v, 5);

    QMF_SORTF(p[0], p[1]);
    QMF_SORTF(p[3], p[4]);
    QMF_SORTF(p[0], p[3]);
    QMF_SORTF(p[1], p[4]);
    QMF_SORTF(p[1], p[2]);
    QMF_SORTF(p[2], p[3]);
    QMF_SORTF(p[1], p[2]);
    return p[2];
}

float quickMedianFilter7f(float *v)
{
    float p[7];
    QMF_COPY(p, v, 7);

    QMF_SORTF(p[0], p[5]);
    QMF_SORTF(p[0], p[3]);
    QMF_SORTF(p[1], p[6]);
    QMF_SORTF(p[2], p[4]);
    QMF_SORTF(p[0], p[1]);
    QMF_SORTF(p[3], p[5]);
    QMF_SORTF(p[2], p[6]);
    QMF_SORTF(p[2], p[3]);
    QMF_SORTF(p[3], p[6]);
    QMF_SORTF(p[4], p[5]);
    QMF_SORTF(p[1], p[4]);
    QMF_SORTF(p[1], p[3]);
    QMF_SORTF(p[3], p[4]);
    return p[3];
}

float quickMedianFilter9f(float *v)
{
    float p[9];
    QMF_COPY(p, v, 9);

    QMF_SORTF(p[1], p[2]);
    QMF_SORTF(p[4], p[5]);
    QMF_SORTF(p[7], p[8]);
    QMF_SORTF(p[0], p[1]);
    QMF_SORTF(p[3], p[4]);
    QMF_SORTF(p[6], p[7]);
    QMF_SORTF(p[1], p[2]);
    QMF_SORTF(p[4], p[5]);
    QMF_SORTF(p[7], p[8]);
    QMF_SORTF(p[0], p[3]);
    QMF_SORTF(p[5], p[8]);
    QMF_SORTF(p[4], p[7]);
    QMF_SORTF(p[3], p[6]);
    QMF_SORTF(p[1], p[4]);
    QMF_SORTF(p[2], p[5]);
    QMF_SORTF(p[4], p[7]);
    QMF_SORTF(p[4], p[2]);
    QMF_SORTF(p[6], p[4]);
    QMF_SORTF(p[4], p[2]);
    return p[4];
}

void arraySubInt32(int32_t *dest, int32_t *array1, int32_t *array2, int count)
{
    for (int i = 0; i < count; i++)
    {
        dest[i] = array1[i] - array2[i];
    }
}

int16_t qPercent(fix12_t q)
{
    return (100 * q) >> 12;
}

int16_t qMultiply(fix12_t q, int16_t input)
{
    return (input * q) >> 12;
}

fix12_t qConstruct(int16_t num, int16_t den)
{
    return (num << 12) / den;
}
