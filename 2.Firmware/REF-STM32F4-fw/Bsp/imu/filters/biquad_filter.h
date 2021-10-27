#ifndef __FILTER_H
#define __FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

struct filter_s;
typedef struct filter_s filter_t;

typedef struct pt1Filter_s
{
    float state;
    float k;
} pt1Filter_t;

typedef struct pt2Filter_s
{
    float state;
    float state1;
    float k;
} pt2Filter_t;

typedef struct pt3Filter_s
{
    float state;
    float state1;
    float state2;
    float k;
} pt3Filter_t;

typedef struct slewFilter_s
{
    float state;
    float slewLimit;
    float threshold;
} slewFilter_t;

/* this holds the data required to update samples thru a filter */
typedef struct biquadFilter_s
{
    float b0, b1, b2, a1, a2;
    float x1, x2, y1, y2;
} BiquadFilter_t;

typedef struct laggedMovingAverage_s
{
    uint16_t movingWindowIndex;
    uint16_t windowSize;
    float movingSum;
    float *buf;
    uint8_t primed;
} laggedMovingAverage_t;

typedef enum
{
    FILTER_PT1 = 0,
    FILTER_BIQUAD,
    FILTER_PT2,
    FILTER_PT3,
} lowpassFilterType_e;

typedef enum
{
    FILTER_LPF,    // 2nd order Butterworth section
    FILTER_NOTCH,
    FILTER_BPF,
} biquadFilterType_e;

typedef float (*filterApplyFnPtr)(filter_t *filter, float input);

float nullFilterApply(filter_t *filter, float input);

void biquadFilterInitLPF(BiquadFilter_t *filter, float _filterCallFreq, float _cutoffFreq);

void biquadFilterInit(BiquadFilter_t *filter, float _filterCallFreq, float _cutoffFreq, float Q,
                      biquadFilterType_e filterType);

void biquadFilterUpdate(BiquadFilter_t *filter,float _filterCallFreq, float _cutoffFreq, float Q,
                        biquadFilterType_e filterType);

void biquadFilterUpdateLPF(BiquadFilter_t *filter, float filterFreq, float refreshRate);

float biquadFilterApplyDF1(BiquadFilter_t *filter, float input);

float biquadFilterApply(BiquadFilter_t *filter, float input);

float filterGetNotchQ(float centerFreq, float cutoffFreq);

void laggedMovingAverageInit(laggedMovingAverage_t *filter, uint16_t windowSize, float *buf);

float laggedMovingAverageUpdate(laggedMovingAverage_t *filter, float input);

float pt1FilterGain(float f_cut, float dT);

void pt1FilterInit(pt1Filter_t *filter, float k);

void pt1FilterUpdateCutoff(pt1Filter_t *filter, float k);

float pt1FilterApply(pt1Filter_t *filter, float input);

float pt2FilterGain(float f_cut, float dT);

void pt2FilterInit(pt2Filter_t *filter, float k);

void pt2FilterUpdateCutoff(pt2Filter_t *filter, float k);

float pt2FilterApply(pt2Filter_t *filter, float input);

float pt3FilterGain(float f_cut, float dT);

void pt3FilterInit(pt3Filter_t *filter, float k);

void pt3FilterUpdateCutoff(pt3Filter_t *filter, float k);

float pt3FilterApply(pt3Filter_t *filter, float input);

void slewFilterInit(slewFilter_t *filter, float slewLimit, float threshold);

float slewFilterApply(slewFilter_t *filter, float input);


#ifdef __cplusplus
}
#endif
#endif
