#include "6dof_kinematic.h"

inline float cosf(float x)
{
    return arm_cos_f32(x);
}

inline float sinf(float x)
{
    return arm_sin_f32(x);
}

static void MatMultiply(const float* _matrix1, const float* _matrix2, float* _matrixOut,
                        const int _m, const int _l, const int _n)
{
    float tmp;
    int i, j, k;
    for (i = 0; i < _m; i++)
    {
        for (j = 0; j < _n; j++)
        {
            tmp = 0.0f;
            for (k = 0; k < _l; k++)
            {
                tmp += _matrix1[_l * i + k] * _matrix2[_n * k + j];
            }
            _matrixOut[_n * i + j] = tmp;
        }
    }
}

static void RotMatToEulerAngle(const float* _rotationM, float* _eulerAngles)
{
    float A, B, C, cb;

    if (fabs(_rotationM[6]) >= 1.0 - 0.0001)
    {
        if (_rotationM[6] < 0)
        {
            A = 0.0f;
            B = (float) M_PI_2;
            C = atan2f(_rotationM[1], _rotationM[4]);
        } else
        {
            A = 0.0f;
            B = -(float) M_PI_2;
            C = -atan2f(_rotationM[1], _rotationM[4]);
        }
    } else
    {
        B = atan2f(-_rotationM[6], sqrtf(_rotationM[0] * _rotationM[0] + _rotationM[3] * _rotationM[3]));
        cb = cosf(B);
        A = atan2f(_rotationM[3] / cb, _rotationM[0] / cb);
        C = atan2f(_rotationM[7] / cb, _rotationM[8] / cb);
    }

    _eulerAngles[0] = C;
    _eulerAngles[1] = B;
    _eulerAngles[2] = A;
}

static void EulerAngleToRotMat(const float* _eulerAngles, float* _rotationM)
{
    float ca, cb, cc, sa, sb, sc;

    cc = cosf(_eulerAngles[0]);
    cb = cosf(_eulerAngles[1]);
    ca = cosf(_eulerAngles[2]);
    sc = sinf(_eulerAngles[0]);
    sb = sinf(_eulerAngles[1]);
    sa = sinf(_eulerAngles[2]);

    _rotationM[0] = ca * cb;
    _rotationM[1] = ca * sb * sc - sa * cc;
    _rotationM[2] = ca * sb * cc + sa * sc;
    _rotationM[3] = sa * cb;
    _rotationM[4] = sa * sb * sc + ca * cc;
    _rotationM[5] = sa * sb * cc - ca * sc;
    _rotationM[6] = -sb;
    _rotationM[7] = cb * sc;
    _rotationM[8] = cb * cc;
}


DOF6Kinematic::DOF6Kinematic(float L_BS, float D_BS, float L_AM, float L_FA, float D_EW, float L_WT)
    : armConfig(ArmConfig_t{L_BS, D_BS, L_AM, L_FA, D_EW, L_WT})
{
    float tmp_DH_matrix[6][4] = {
        {0.0f,            armConfig.L_BASE,    armConfig.D_BASE, -(float) M_PI_2},
        {-(float) M_PI_2, 0.0f,                armConfig.L_ARM,  0.0f},
        {(float) M_PI_2,  armConfig.D_ELBOW,   0.0f,             (float) M_PI_2},
        {0.0f,            armConfig.L_FOREARM, 0.0f,             -(float) M_PI_2},
        {0.0f,            0.0f,                0.0f,             (float) M_PI_2},
        {0.0f,            armConfig.L_WRIST, 0.0f, 0.0f}
    };
    memcpy(DH_matrix, tmp_DH_matrix, sizeof(tmp_DH_matrix));

    float tmp_L1_bs[3] = {armConfig.D_BASE, -armConfig.L_BASE, 0.0f};
    memcpy(L1_base, tmp_L1_bs, sizeof(tmp_L1_bs));
    float tmp_L2_se[3] = {armConfig.L_ARM, 0.0f, 0.0f};
    memcpy(L2_arm, tmp_L2_se, sizeof(tmp_L2_se));
    float tmp_L3_ew[3] = {-armConfig.D_ELBOW, 0.0f, armConfig.L_FOREARM};
    memcpy(L3_elbow, tmp_L3_ew, sizeof(tmp_L3_ew));
    float tmp_L6_wt[3] = {0.0f, 0.0f, armConfig.L_WRIST};
    memcpy(L6_wrist, tmp_L6_wt, sizeof(tmp_L6_wt));

    l_se_2 = armConfig.L_ARM * armConfig.L_ARM;
    l_se = armConfig.L_ARM;
    l_ew_2 = armConfig.L_FOREARM * armConfig.L_FOREARM + armConfig.D_ELBOW * armConfig.D_ELBOW;
    l_ew = 0;
    atan_e = 0;
}

bool
DOF6Kinematic::SolveFK(const DOF6Kinematic::Joint6D_t &_inputJoint6D, DOF6Kinematic::Pose6D_t &_outputPose6D)
{
    float q_in[6];
    float q[6];
    float cosq, sinq;
    float cosa, sina;
    float P06[6];
    float R06[9];
    float R[6][9];
    float R02[9];
    float R03[9];
    float R04[9];
    float R05[9];
    float L0_bs[3];
    float L0_se[3];
    float L0_ew[3];
    float L0_wt[3];

    for (int i = 0; i < 6; i++)
        q_in[i] = _inputJoint6D.a[i] / RAD_TO_DEG;

    for (int i = 0; i < 6; i++)
    {
        q[i] = q_in[i] + DH_matrix[i][0];
        cosq = cosf(q[i]);
        sinq = sinf(q[i]);
        cosa = cosf(DH_matrix[i][3]);
        sina = sinf(DH_matrix[i][3]);

        R[i][0] = cosq;
        R[i][1] = -cosa * sinq;
        R[i][2] = sina * sinq;
        R[i][3] = sinq;
        R[i][4] = cosa * cosq;
        R[i][5] = -sina * cosq;
        R[i][6] = 0.0f;
        R[i][7] = sina;
        R[i][8] = cosa;
    }

    MatMultiply(R[0], R[1], R02, 3, 3, 3);
    MatMultiply(R02, R[2], R03, 3, 3, 3);
    MatMultiply(R03, R[3], R04, 3, 3, 3);
    MatMultiply(R04, R[4], R05, 3, 3, 3);
    MatMultiply(R05, R[5], R06, 3, 3, 3);

    MatMultiply(R[0], L1_base, L0_bs, 3, 3, 1);
    MatMultiply(R02, L2_arm, L0_se, 3, 3, 1);
    MatMultiply(R03, L3_elbow, L0_ew, 3, 3, 1);
    MatMultiply(R06, L6_wrist, L0_wt, 3, 3, 1);

    for (int i = 0; i < 3; i++)
        P06[i] = L0_bs[i] + L0_se[i] + L0_ew[i] + L0_wt[i];

    RotMatToEulerAngle(R06, &(P06[3]));

    _outputPose6D.X = P06[0];
    _outputPose6D.Y = P06[1];
    _outputPose6D.Z = P06[2];
    _outputPose6D.A = P06[3] * RAD_TO_DEG;
    _outputPose6D.B = P06[4] * RAD_TO_DEG;
    _outputPose6D.C = P06[5] * RAD_TO_DEG;
    memcpy(_outputPose6D.R, R06, 9 * sizeof(float));

    return true;
}

bool DOF6Kinematic::SolveIK(const DOF6Kinematic::Pose6D_t &_inputPose6D, const Joint6D_t &_lastJoint6D,
                            DOF6Kinematic::IKSolves_t &_outputSolves)
{
    float qs[2];
    float qa[2][2];
    float qw[2][3];
    float cosqs, sinqs;
    float cosqa[2], sinqa[2];
    float cosqw, sinqw;
    float P06[6];
    float R06[9];
    float P0_w[3];
    float P1_w[3];
    float L0_wt[3];
    float L1_sw[3];
    float R10[9];
    float R31[9];
    float R30[9];
    float R36[9];
    float l_sw_2, l_sw, atan_a, acos_a, acos_e;

    int ind_arm, ind_elbow, ind_wrist;
    int i;

    if (0 == l_ew)
    {
        l_ew = sqrtf(l_ew_2);
        atan_e = atanf(armConfig.D_ELBOW / armConfig.L_FOREARM);
    }

    P06[0] = _inputPose6D.X / 1000.0f;
    P06[1] = _inputPose6D.Y / 1000.0f;
    P06[2] = _inputPose6D.Z / 1000.0f;
    if (!_inputPose6D.hasR)
    {
        P06[3] = _inputPose6D.A / RAD_TO_DEG;
        P06[4] = _inputPose6D.B / RAD_TO_DEG;
        P06[5] = _inputPose6D.C / RAD_TO_DEG;
        EulerAngleToRotMat(&(P06[3]), R06);
    } else
    {
        memcpy(R06, _inputPose6D.R, 9 * sizeof(float));
    }
    for (i = 0; i < 2; i++)
    {
        qs[i] = _lastJoint6D.a[0];
        qa[i][0] = _lastJoint6D.a[1];
        qa[i][1] = _lastJoint6D.a[2];
        qw[i][0] = _lastJoint6D.a[3];
        qw[i][1] = _lastJoint6D.a[4];
        qw[i][2] = _lastJoint6D.a[5];
    }
    MatMultiply(R06, L6_wrist, L0_wt, 3, 3, 1);
    for (i = 0; i < 3; i++)
    {
        P0_w[i] = P06[i] - L0_wt[i];
    }
    if (sqrt(P0_w[0] * P0_w[0] + P0_w[1] * P0_w[1]) <= 0.000001)
    {
        qs[0] = _lastJoint6D.a[0];
        qs[1] = _lastJoint6D.a[0];
        for (i = 0; i < 4; i++)
        {
            _outputSolves.solFlag[0 + i][0] = -1;
            _outputSolves.solFlag[4 + i][0] = -1;
        }
    } else
    {
        qs[0] = atan2f(P0_w[1], P0_w[0]);
        qs[1] = atan2f(-P0_w[1], -P0_w[0]);
        for (i = 0; i < 4; i++)
        {
            _outputSolves.solFlag[0 + i][0] = 1;
            _outputSolves.solFlag[4 + i][0] = 1;
        }
    }
    for (ind_arm = 0; ind_arm < 2; ind_arm++)
    {
        cosqs = cosf(qs[ind_arm] + DH_matrix[0][0]);
        sinqs = sinf(qs[ind_arm] + DH_matrix[0][0]);

        R10[0] = cosqs;
        R10[1] = sinqs;
        R10[2] = 0.0f;
        R10[3] = 0.0f;
        R10[4] = 0.0f;
        R10[5] = -1.0f;
        R10[6] = -sinqs;
        R10[7] = cosqs;
        R10[8] = 0.0f;

        MatMultiply(R10, P0_w, P1_w, 3, 3, 1);
        for (i = 0; i < 3; i++)
        {
            L1_sw[i] = P1_w[i] - L1_base[i];
        }
        l_sw_2 = L1_sw[0] * L1_sw[0] + L1_sw[1] * L1_sw[1];
        l_sw = sqrtf(l_sw_2);

        if (fabs(l_se + l_ew - l_sw) <= 0.000001)
        {
            qa[0][0] = atan2f(L1_sw[1], L1_sw[0]);
            qa[1][0] = qa[0][0];
            qa[0][1] = 0.0f;
            qa[1][1] = 0.0f;
            if (l_sw > l_se + l_ew)
            {
                for (i = 0; i < 2; i++)
                {
                    _outputSolves.solFlag[4 * ind_arm + 0 + i][1] = 0;
                    _outputSolves.solFlag[4 * ind_arm + 2 + i][1] = 0;
                }
            } else
            {
                for (i = 0; i < 2; i++)
                {
                    _outputSolves.solFlag[4 * ind_arm + 0 + i][1] = 1;
                    _outputSolves.solFlag[4 * ind_arm + 2 + i][1] = 1;
                }
            }
        } else if (fabs(l_sw - fabs(l_se - l_ew)) <= 0.000001)
        {
            qa[0][0] = atan2f(L1_sw[1], L1_sw[0]);
            qa[1][0] = qa[0][0];
            if (0 == ind_arm)
            {
                qa[0][1] = (float) M_PI;
                qa[1][1] = -(float) M_PI;
            } else
            {
                qa[0][1] = -(float) M_PI;
                qa[1][1] = (float) M_PI;
            }
            if (l_sw < fabs(l_se - l_ew))
            {
                for (i = 0; i < 2; i++)
                {
                    _outputSolves.solFlag[4 * ind_arm + 0 + i][1] = 0;
                    _outputSolves.solFlag[4 * ind_arm + 2 + i][1] = 0;
                }
            } else
            {
                for (i = 0; i < 2; i++)
                {
                    _outputSolves.solFlag[4 * ind_arm + 0 + i][1] = 1;
                    _outputSolves.solFlag[4 * ind_arm + 2 + i][1] = 1;
                }
            }
        } else
        {
            atan_a = atan2f(L1_sw[1], L1_sw[0]);
            acos_a = 0.5f * (l_se_2 + l_sw_2 - l_ew_2) / (l_se * l_sw);
            if (acos_a >= 1.0f) acos_a = 0.0f;
            else if (acos_a <= -1.0f) acos_a = (float) M_PI;
            else acos_a = acosf(acos_a);
            acos_e = 0.5f * (l_se_2 + l_ew_2 - l_sw_2) / (l_se * l_ew);
            if (acos_e >= 1.0f) acos_e = 0.0f;
            else if (acos_e <= -1.0f) acos_e = (float) M_PI;
            else acos_e = acosf(acos_e);
            if (0 == ind_arm)
            {
                qa[0][0] = atan_a - acos_a + (float) M_PI_2;
                qa[0][1] = atan_e - acos_e + (float) M_PI;
                qa[1][0] = atan_a + acos_a + (float) M_PI_2;
                qa[1][1] = atan_e + acos_e - (float) M_PI;

            } else
            {
                qa[0][0] = atan_a + acos_a + (float) M_PI_2;
                qa[0][1] = atan_e + acos_e - (float) M_PI;
                qa[1][0] = atan_a - acos_a + (float) M_PI_2;
                qa[1][1] = atan_e - acos_e + (float) M_PI;
            }
            for (i = 0; i < 2; i++)
            {
                _outputSolves.solFlag[4 * ind_arm + 0 + i][1] = 1;
                _outputSolves.solFlag[4 * ind_arm + 2 + i][1] = 1;
            }
        }
        for (ind_elbow = 0; ind_elbow < 2; ind_elbow++)
        {
            cosqa[0] = cosf(qa[ind_elbow][0] + DH_matrix[1][0]);
            sinqa[0] = sinf(qa[ind_elbow][0] + DH_matrix[1][0]);
            cosqa[1] = cosf(qa[ind_elbow][1] + DH_matrix[2][0]);
            sinqa[1] = sinf(qa[ind_elbow][1] + DH_matrix[2][0]);

            R31[0] = cosqa[0] * cosqa[1] - sinqa[0] * sinqa[1];
            R31[1] = cosqa[0] * sinqa[1] + sinqa[0] * cosqa[1];
            R31[2] = 0.0f;
            R31[3] = 0.0f;
            R31[4] = 0.0f;
            R31[5] = 1.0f;
            R31[6] = cosqa[0] * sinqa[1] + sinqa[0] * cosqa[1];
            R31[7] = -cosqa[0] * cosqa[1] + sinqa[0] * sinqa[1];
            R31[8] = 0.0f;

            MatMultiply(R31, R10, R30, 3, 3, 3);
            MatMultiply(R30, R06, R36, 3, 3, 3);

            if (R36[8] >= 1.0 - 0.000001)
            {
                cosqw = 1.0f;
                qw[0][1] = 0.0f;
                qw[1][1] = 0.0f;
            } else if (R36[8] <= -1.0 + 0.000001)
            {
                cosqw = -1.0f;
                if (0 == ind_arm)
                {
                    qw[0][1] = (float) M_PI;
                    qw[1][1] = -(float) M_PI;
                } else
                {
                    qw[0][1] = -(float) M_PI;
                    qw[1][1] = (float) M_PI;
                }
            } else
            {
                cosqw = R36[8];
                if (0 == ind_arm)
                {
                    qw[0][1] = acosf(cosqw);
                    qw[1][1] = -acosf(cosqw);
                } else
                {
                    qw[0][1] = -acosf(cosqw);
                    qw[1][1] = acosf(cosqw);
                }
            }
            if (1.0f == cosqw || -1.0f == cosqw)
            {
                if (0 == ind_arm)
                {
                    qw[0][0] = _lastJoint6D.a[3];
                    cosqw = cosf(_lastJoint6D.a[3] + DH_matrix[3][0]);
                    sinqw = sinf(_lastJoint6D.a[3] + DH_matrix[3][0]);
                    qw[0][2] = atan2f(cosqw * R36[3] - sinqw * R36[0], cosqw * R36[0] + sinqw * R36[3]);
                    qw[1][2] = _lastJoint6D.a[5];
                    cosqw = cosf(_lastJoint6D.a[5] + DH_matrix[5][0]);
                    sinqw = sinf(_lastJoint6D.a[5] + DH_matrix[5][0]);
                    qw[1][0] = atan2f(cosqw * R36[3] - sinqw * R36[0], cosqw * R36[0] + sinqw * R36[3]);
                } else
                {
                    qw[0][2] = _lastJoint6D.a[5];
                    cosqw = cosf(_lastJoint6D.a[5] + DH_matrix[5][0]);
                    sinqw = sinf(_lastJoint6D.a[5] + DH_matrix[5][0]);
                    qw[0][0] = atan2f(cosqw * R36[3] - sinqw * R36[0], cosqw * R36[0] + sinqw * R36[3]);
                    qw[1][0] = _lastJoint6D.a[3];
                    cosqw = cosf(_lastJoint6D.a[3] + DH_matrix[3][0]);
                    sinqw = sinf(_lastJoint6D.a[3] + DH_matrix[3][0]);
                    qw[1][2] = atan2f(cosqw * R36[3] - sinqw * R36[0], cosqw * R36[0] + sinqw * R36[3]);
                }
                _outputSolves.solFlag[4 * ind_arm + 2 * ind_elbow + 0][2] = -1;
                _outputSolves.solFlag[4 * ind_arm + 2 * ind_elbow + 1][2] = -1;
            } else
            {
                if (0 == ind_arm)
                {
                    qw[0][0] = atan2f(R36[5], R36[2]);
                    qw[1][0] = atan2f(-R36[5], -R36[2]);
                    qw[0][2] = atan2f(R36[7], -R36[6]);
                    qw[1][2] = atan2f(-R36[7], R36[6]);
                } else
                {
                    qw[0][0] = atan2f(-R36[5], -R36[2]);
                    qw[1][0] = atan2f(R36[5], R36[2]);
                    qw[0][2] = atan2f(-R36[7], R36[6]);
                    qw[1][2] = atan2f(R36[7], -R36[6]);
                }
                _outputSolves.solFlag[4 * ind_arm + 2 * ind_elbow + 0][2] = 1;
                _outputSolves.solFlag[4 * ind_arm + 2 * ind_elbow + 1][2] = 1;
            }
            for (ind_wrist = 0; ind_wrist < 2; ind_wrist++)
            {
                if (qs[ind_arm] > (float) M_PI)
                    _outputSolves.config[4 * ind_arm + 2 * ind_elbow + ind_wrist].a[0] =
                        qs[ind_arm] - (float) M_PI;
                else if (qs[ind_arm] < -(float) M_PI)
                    _outputSolves.config[4 * ind_arm + 2 * ind_elbow + ind_wrist].a[0] =
                        qs[ind_arm] + (float) M_PI;
                else
                    _outputSolves.config[4 * ind_arm + 2 * ind_elbow + ind_wrist].a[0] = qs[ind_arm];

                for (i = 0; i < 2; i++)
                {
                    if (qa[ind_elbow][i] > (float) M_PI)
                        _outputSolves.config[4 * ind_arm + 2 * ind_elbow + ind_wrist].a[1 + i] =
                            qa[ind_elbow][i] - (float) M_PI;
                    else if (qa[ind_elbow][i] < -(float) M_PI)
                        _outputSolves.config[4 * ind_arm + 2 * ind_elbow + ind_wrist].a[1 + i] =
                            qa[ind_elbow][i] + (float) M_PI;
                    else
                        _outputSolves.config[4 * ind_arm + 2 * ind_elbow + ind_wrist].a[1 + i] =
                            qa[ind_elbow][i];
                }

                for (i = 0; i < 3; i++)
                {
                    if (qw[ind_wrist][i] > (float) M_PI)
                        _outputSolves.config[4 * ind_arm + 2 * ind_elbow + ind_wrist].a[3 + i] =
                            qw[ind_wrist][i] - (float) M_PI;
                    else if (qw[ind_wrist][i] < -(float) M_PI)
                        _outputSolves.config[4 * ind_arm + 2 * ind_elbow + ind_wrist].a[3 + i] =
                            qw[ind_wrist][i] + (float) M_PI;
                    else
                        _outputSolves.config[4 * ind_arm + 2 * ind_elbow + ind_wrist].a[3 + i] =
                            qw[ind_wrist][i];
                }
            }
        }
    }

    for (i = 0; i < 8; i++)
        for (float &j: _outputSolves.config[i].a)
            j *= RAD_TO_DEG;

    return true;
}

DOF6Kinematic::Joint6D_t
operator-(const DOF6Kinematic::Joint6D_t &_joints1, const DOF6Kinematic::Joint6D_t &_joints2)
{
    DOF6Kinematic::Joint6D_t tmp{};
    for (int i = 0; i < 6; i++)
        tmp.a[i] = _joints1.a[i] - _joints2.a[i];

    return tmp;
}
