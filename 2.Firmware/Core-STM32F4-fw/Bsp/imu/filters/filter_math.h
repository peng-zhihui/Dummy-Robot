#ifndef __MATHS_H
#define __MATHS_H
#ifdef __cplusplus
extern "C" {
#endif

#ifndef sq
#define sq(x) ((x)*(x))
#endif
#define power3(x) ((x)*(x)*(x))

// Undefine this for use libc sinf/cosf. Keep this defined to use fast sin/cos approximations
//#define FAST_MATH             // order 9 approximation
#define VERY_FAST_MATH        // order 7 approximation

// Use floating point M_PI instead explicitly.
#define M_PIf       3.14159265358979323846f
#define M_EULERf    2.71828182845904523536f

#define RAD    (M_PIf / 180.0f)
#define DEGREES_TO_DECIDEGREES(angle) ((angle) * 10)
#define DECIDEGREES_TO_DEGREES(angle) ((angle) / 10)
#define DECIDEGREES_TO_RADIANS(angle) ((angle) / 10.0f * 0.0174532925f)
#define DEGREES_TO_RADIANS(angle) ((angle) * 0.0174532925f)

#define CM_S_TO_KM_H(centimetersPerSecond) ((centimetersPerSecond) * 36 / 1000)
#define CM_S_TO_MPH(centimetersPerSecond) ((centimetersPerSecond) * 10000 / 5080 / 88)

#define MIN(a, b) \
  __extension__ ({ __typeof__ (a) _a = (a); \
  __typeof__ (b) _b = (b); \
  _a < _b ? _a : _b; })
#define MAX(a, b) \
  __extension__ ({ __typeof__ (a) _a = (a); \
  __typeof__ (b) _b = (b); \
  _a > _b ? _a : _b; })
#define ABS(x) \
  __extension__ ({ __typeof__ (x) _x = (x); \
  _x > 0 ? _x : -_x; })

#define Q12 (1 << 12)

#define HZ_TO_INTERVAL(x) (1.0f / (x))
#define HZ_TO_INTERVAL_US(x) (1000000 / (x))

typedef int32_t fix12_t;

typedef struct stdev_s
{
    float m_oldM, m_newM, m_oldS, m_newS;
    int m_n;
} stdev_t;

// Floating point 3 vector.
typedef struct fp_vector
{
    float X;
    float Y;
    float Z;
} t_fp_vector_def;

typedef union u_fp_vector
{
    float A[3];
    t_fp_vector_def V;
} t_fp_vector;

// Floating point Euler angles.
// Be carefull, could be either of degrees or radians.
typedef struct fp_angles
{
    float roll;
    float pitch;
    float yaw;
} fp_angles_def;

typedef union
{
    float raw[3];
    fp_angles_def angles;
} fp_angles_t;

typedef struct fp_rotationMatrix_s
{
    float m[3][3];              // matrix
} fp_rotationMatrix_t;

int gcd(int num, int denom);

int32_t applyDeadband(int32_t value, int32_t deadband);

float fapplyDeadband(float value, float deadband);

void devClear(stdev_t *dev);

void devPush(stdev_t *dev, float x);

float devVariance(stdev_t *dev);

float devStandardDeviation(stdev_t *dev);

float degreesToRadians(int16_t degrees);

int scaleRange(int x, int srcFrom, int srcTo, int destFrom, int destTo);

float scaleRangef(float x, float srcFrom, float srcTo, float destFrom, float destTo);

int32_t quickMedianFilter3(int32_t *v);

int32_t quickMedianFilter5(int32_t *v);

int32_t quickMedianFilter7(int32_t *v);

int32_t quickMedianFilter9(int32_t *v);

float quickMedianFilter3f(float *v);

float quickMedianFilter5f(float *v);

float quickMedianFilter7f(float *v);

float quickMedianFilter9f(float *v);

#if defined(FAST_MATH) || defined(VERY_FAST_MATH)

float sin_approx(float x);

float cos_approx(float x);

float atan2_approx(float y, float x);

float acos_approx(float x);

#define tan_approx(x)       (sin_approx(x) / cos_approx(x))

float exp_approx(float val);

float log_approx(float val);

float pow_approx(float a, float b);

#else
#define sin_approx(x)       sinf(x)
#define cos_approx(x)       cosf(x)
#define atan2_approx(y,x)   atan2f(y,x)
#define acos_approx(x)      acosf(x)
#define tan_approx(x)       tanf(x)
#define exp_approx(x)       expf(x)
#define log_approx(x)       logf(x)
#define pow_approx(a, b)    powf(b, a)
#endif

void arraySubInt32(int32_t *dest, int32_t *array1, int32_t *array2, int count);

int16_t qPercent(fix12_t q);

int16_t qMultiply(fix12_t q, int16_t input);

fix12_t qConstruct(int16_t num, int16_t den);

static inline int constrain(int amt, int low, int high)
{
    if (amt < low)
        return low;
    else if (amt > high)
        return high;
    else
        return amt;
}

static inline float constrainf(float amt, float low, float high)
{
    if (amt < low)
        return low;
    else if (amt > high)
        return high;
    else
        return amt;
}

#ifdef __cplusplus
}
#endif
#endif