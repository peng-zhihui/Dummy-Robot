#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "biquad_filter.h"
#include "filter_math.h"

#define BIQUAD_Q 1.0f / sqrtf(2.0f)     /* quality factor - 2nd order butterworth*/

// NULL filter

float nullFilterApply(filter_t *filter, float input)
{
    (void) (filter);
    return input;
}


// PT1 Low Pass filter

float pt1FilterGain(float f_cut, float dT)
{
    float RC = 1 / (2 * M_PIf * f_cut);
    return dT / (RC + dT);
}

void pt1FilterInit(pt1Filter_t *filter, float k)
{
    filter->state = 0.0f;
    filter->k = k;
}

void pt1FilterUpdateCutoff(pt1Filter_t *filter, float k)
{
    filter->k = k;
}

float pt1FilterApply(pt1Filter_t *filter, float input)
{
    filter->state = filter->state + filter->k * (input - filter->state);
    return filter->state;
}

// PT2 Low Pass filter

float pt2FilterGain(float f_cut, float dT)
{
    const float order = 2.0f;
    const float orderCutoffCorrection = 1 / sqrtf(powf(2, 1.0f / order) - 1);
    float RC = 1 / (2 * orderCutoffCorrection * M_PIf * f_cut);
    // float RC = 1 / (2 * 1.553773974f * M_PIf * f_cut);
    // where 1.553773974 = 1 / sqrt( (2^(1 / order) - 1) ) and order is 2
    return dT / (RC + dT);
}

void pt2FilterInit(pt2Filter_t *filter, float k)
{
    filter->state = 0.0f;
    filter->state1 = 0.0f;
    filter->k = k;
}

void pt2FilterUpdateCutoff(pt2Filter_t *filter, float k)
{
    filter->k = k;
}

float pt2FilterApply(pt2Filter_t *filter, float input)
{
    filter->state1 = filter->state1 + filter->k * (input - filter->state1);
    filter->state = filter->state + filter->k * (filter->state1 - filter->state);
    return filter->state;
}

// PT3 Low Pass filter

float pt3FilterGain(float f_cut, float dT)
{
    const float order = 3.0f;
    const float orderCutoffCorrection = 1 / sqrtf(powf(2, 1.0f / order) - 1);
    float RC = 1 / (2 * orderCutoffCorrection * M_PIf * f_cut);
    // float RC = 1 / (2 * 1.961459177f * M_PIf * f_cut);
    // where 1.961459177 = 1 / sqrt( (2^(1 / order) - 1) ) and order is 3
    return dT / (RC + dT);
}

void pt3FilterInit(pt3Filter_t *filter, float k)
{
    filter->state = 0.0f;
    filter->state1 = 0.0f;
    filter->state2 = 0.0f;
    filter->k = k;
}

void pt3FilterUpdateCutoff(pt3Filter_t *filter, float k)
{
    filter->k = k;
}

float pt3FilterApply(pt3Filter_t *filter, float input)
{
    filter->state1 = filter->state1 + filter->k * (input - filter->state1);
    filter->state2 = filter->state2 + filter->k * (filter->state1 - filter->state2);
    filter->state = filter->state + filter->k * (filter->state2 - filter->state);
    return filter->state;
}


// Slew filter with limit

void slewFilterInit(slewFilter_t *filter, float slewLimit, float threshold)
{
    filter->state = 0.0f;
    filter->slewLimit = slewLimit;
    filter->threshold = threshold;
}

float slewFilterApply(slewFilter_t *filter, float input)
{
    if (filter->state >= filter->threshold)
    {
        if (input >= filter->state - filter->slewLimit)
        {
            filter->state = input;
        }
    } else if (filter->state <= -filter->threshold)
    {
        if (input <= filter->state + filter->slewLimit)
        {
            filter->state = input;
        }
    } else
    {
        filter->state = input;
    }
    return filter->state;
}

// get notch filter Q given center frequency (f0) and lower cutoff frequency (f1)
// Q = f0 / (f2 - f1) ; f2 = f0^2 / f1
float filterGetNotchQ(float centerFreq, float cutoffFreq)
{
    return centerFreq * cutoffFreq / (centerFreq * centerFreq - cutoffFreq * cutoffFreq);
}

/* sets up a biquad filter as a 2nd order butterworth LPF */
void biquadFilterInitLPF(BiquadFilter_t *filter, float _filterCallFreq, float _cutoffFreq)
{
    biquadFilterInit(filter, _filterCallFreq, _cutoffFreq, BIQUAD_Q, FILTER_LPF);
}

void
biquadFilterInit(BiquadFilter_t *filter, float _filterCallFreq, float _cutoffFreq, float Q,
                 biquadFilterType_e filterType)
{
    biquadFilterUpdate(filter, _filterCallFreq, _cutoffFreq, Q, filterType);

    // zero initial samples
    filter->x1 = filter->x2 = 0;
    filter->y1 = filter->y2 = 0;
}

void biquadFilterUpdate(BiquadFilter_t *filter, float _filterCallFreq, float _cutoffFreq, float Q,
                        biquadFilterType_e filterType)
{
    // setup variables
    const float omega = 2.0f * M_PIf * _filterCallFreq * _cutoffFreq * 0.000001f;
    const float sn = sin_approx(omega);
    const float cs = cos_approx(omega);
    const float alpha = sn / (2.0f * Q);

    switch (filterType)
    {
        case FILTER_LPF:
            // 2nd order Butterworth (with Q=1/sqrt(2)) / Butterworth biquad section with Q
            // described in http://www.ti.com/lit/an/slaa447/slaa447.pdf
            filter->b1 = 1 - cs;
            filter->b0 = filter->b1 * 0.5f;
            filter->b2 = filter->b0;
            filter->a1 = -2 * cs;
            filter->a2 = 1 - alpha;
            break;
        case FILTER_NOTCH:
            filter->b0 = 1;
            filter->b1 = -2 * cs;
            filter->b2 = 1;
            filter->a1 = filter->b1;
            filter->a2 = 1 - alpha;
            break;
        case FILTER_BPF:
            filter->b0 = alpha;
            filter->b1 = 0;
            filter->b2 = -alpha;
            filter->a1 = -2 * cs;
            filter->a2 = 1 - alpha;
            break;
    }

    const float a0 = 1 + alpha;

    // precompute the coefficients
    filter->b0 /= a0;
    filter->b1 /= a0;
    filter->b2 /= a0;
    filter->a1 /= a0;
    filter->a2 /= a0;
}

void biquadFilterUpdateLPF(BiquadFilter_t *filter, float filterFreq, float refreshRate)
{
    biquadFilterUpdate(filter, filterFreq, refreshRate, BIQUAD_Q, FILTER_LPF);
}

/* Computes a biquadFilter_t filter on a sample (slightly less precise than df2 but works in dynamic mode) */
float biquadFilterApplyDF1(BiquadFilter_t *filter, float input)
{
    /* compute result */
    const float result =
        filter->b0 * input + filter->b1 * filter->x1 + filter->b2 * filter->x2 - filter->a1 * filter->y1 -
        filter->a2 * filter->y2;

    /* shift x1 to x2, input to x1 */
    filter->x2 = filter->x1;
    filter->x1 = input;

    /* shift y1 to y2, result to y1 */
    filter->y2 = filter->y1;
    filter->y1 = result;

    return result;
}

/* Computes a biquadFilter_t filter in direct form 2 on a sample (higher precision but can't handle changes in coefficients */
float biquadFilterApply(BiquadFilter_t *filter, float input)
{
    const float result = filter->b0 * input + filter->x1;
    filter->x1 = filter->b1 * input - filter->a1 * result + filter->x2;
    filter->x2 = filter->b2 * input - filter->a2 * result;
    return result;
}

void laggedMovingAverageInit(laggedMovingAverage_t *filter, uint16_t windowSize, float *buf)
{
    filter->movingWindowIndex = 0;
    filter->windowSize = windowSize;
    filter->buf = buf;
    filter->movingSum = 0;
    memset(filter->buf, 0, windowSize * sizeof(float));
    filter->primed = false;
}

float laggedMovingAverageUpdate(laggedMovingAverage_t *filter, float input)
{
    filter->movingSum -= filter->buf[filter->movingWindowIndex];
    filter->buf[filter->movingWindowIndex] = input;
    filter->movingSum += input;

    if (++filter->movingWindowIndex == filter->windowSize)
    {
        filter->movingWindowIndex = 0;
        filter->primed = true;
    }

    const uint16_t denom = filter->primed ? filter->windowSize : filter->movingWindowIndex;
    return filter->movingSum / denom;
}
