#ifndef DOF6_KINEMATIC_SOLVER_H
#define DOF6_KINEMATIC_SOLVER_H

#include "stm32f405xx.h"
#include "arm_math.h"
#include "memory.h"

class DOF6Kinematic
{
private:
    const float RAD_TO_DEG = 57.295777754771045f;

    // DH parameters
    struct ArmConfig_t
    {
        float L_BASE;
        float D_BASE;
        float L_ARM;
        float L_FOREARM;
        float D_ELBOW;
        float L_WRIST;
    };
    ArmConfig_t armConfig;

    float DH_matrix[6][4] = {0};
    float L1_BASE[3] = {0};
    float L2_ARM[3] = {0};
    float L3_ELBOW[3] = {0};
    float L6_WRIST[3] = {0};

public:
    struct Joint6D_t
    {
        Joint6D_t()
        = default;

        Joint6D_t(float a1, float a2, float a3, float a4, float a5, float a6)
            : a{a1, a2, a3, a4, a5, a6}
        {}

        float a[6];

        friend Joint6D_t operator-(const Joint6D_t &_joints1, const Joint6D_t &_joints2);
    };

    struct Pose6D_t
    {
        Pose6D_t()
        = default;

        Pose6D_t(float x, float y, float z, float a, float b, float c)
            : X(x), Y(y), Z(z), A(a), B(b), C(c), hasR(false)
        {}

        float X{}, Y{}, Z{};
        float A{}, B{}, C{};
        float R[9]{};

        // if Pose was calculated by FK then it's true automatically (so that no need to do extra calc),
        // otherwise if manually set params then it should be set to false.
        bool hasR{};
    };

    struct IKSolves_t
    {
        Joint6D_t config[8];
        char solFlag[8][3];
    };

    DOF6Kinematic(float L_BASE, float D_BASE, float L_ARM, float L_FOREARM, float D_ELBOW, float L_WRIST);

    bool SolveFK(const Joint6D_t &_inputJoint6D, Pose6D_t &_outputPose6D);

    bool SolveIK(const Pose6D_t &_inputPose6D, const Joint6D_t &_lastJoint6D, IKSolves_t &_outputSolves);
};

#endif //DOF6_KINEMATIC_SOLVER_H
