#include <cmsis_os2.h>
#include <communication.hpp>
#include "dummy_robot.h"

inline float AbsMaxOf6(DOF6Kinematic::Joint6D_t &_joints)
{
    float max = -1;

    for (int i = 0; i < 6; i++)
    {
        if (abs(_joints.a[i]) > max)
            max = abs(_joints.a[i]);
    }

    return max;
}

DummyRobot::DummyRobot(CAN_HandleTypeDef* _hcan) :
    hcan(_hcan)
{
    motorAll = new CtrlStepMotor(_hcan, 0, false, 30, -180, 180);
    motorJ1 = new CtrlStepMotor(_hcan, 1, true, 50, -170 - initPose.a[0], 170 - initPose.a[0]);
    motorJ2 = new CtrlStepMotor(_hcan, 2, true, 30, -75 - initPose.a[1], 90 - initPose.a[1]);
    motorJ3 = new CtrlStepMotor(_hcan, 3, true, 30, 35 - initPose.a[2], 180 - initPose.a[2]);
    motorJ4 = new CtrlStepMotor(_hcan, 4, false, 24, -180 - initPose.a[3], 180 - initPose.a[3]);
    motorJ5 = new CtrlStepMotor(_hcan, 5, true, 30, -120 - initPose.a[4], 120 - initPose.a[4]);
    motorJ6 = new CtrlStepMotor(_hcan, 6, true, 50, -720 - initPose.a[5], 720 - initPose.a[5]);
    hand = new DummyHand(_hcan, 7);

    dof6Solver = new DOF6Kinematic(0.109f, 0.035f, 0.146f, 0.115f, 0.052f, 0.072f);
}

DummyRobot::~DummyRobot()
{
    delete motorAll;
    delete motorJ1;
    delete motorJ2;
    delete motorJ3;
    delete motorJ4;
    delete motorJ5;
    delete motorJ6;
    delete hand;

    delete dof6Solver;
}

void DummyRobot::Reboot()
{
    motorAll->Reboot();
}

float DummyRobot::MoveJoints(DOF6Kinematic::Joint6D_t &_joints)
{
    DOF6Kinematic::Joint6D_t tmp = _joints - currentJoints;
    float maxAngle = AbsMaxOf6(tmp);
    float time = maxAngle / jointSpeed;

    motorJ1->SetAngleWithTime(_joints.a[0] - initPose.a[0], time);
    motorJ2->SetAngleWithTime(_joints.a[1] - initPose.a[1], time);
    motorJ3->SetAngleWithTime(_joints.a[2] - initPose.a[2], time);
    motorJ4->SetAngleWithTime(_joints.a[3] - initPose.a[3], time);
    motorJ5->SetAngleWithTime(_joints.a[4] - initPose.a[4], time);
    motorJ6->SetAngleWithTime(_joints.a[5] - initPose.a[5], time);

//    currentJoints = _joints;

    return time;
}

float DummyRobot::MoveJ(float _j1, float _j2, float _j3, float _j4, float _j5, float _j6)
{
    DOF6Kinematic::Joint6D_t joints(_j1, _j2, _j3, _j4, _j5, _j6);

    bool valid = true;
    if (_j1 - initPose.a[0] > motorJ1->maxAngle ||
        _j1 - initPose.a[0] < motorJ1->minAngle)
        valid = false;
    if (_j2 - initPose.a[1] > motorJ2->maxAngle ||
        _j2 - initPose.a[1] < motorJ2->minAngle)
        valid = false;
    if (_j3 - initPose.a[2] > motorJ3->maxAngle ||
        _j3 - initPose.a[2] < motorJ3->minAngle)
        valid = false;
    if (_j4 - initPose.a[3] > motorJ4->maxAngle ||
        _j4 - initPose.a[3] < motorJ4->minAngle)
        valid = false;
    if (_j5 - initPose.a[4] > motorJ5->maxAngle ||
        _j5 - initPose.a[4] < motorJ5->minAngle)
        valid = false;
    if (_j6 - initPose.a[5] > motorJ6->maxAngle ||
        _j6 - initPose.a[5] < motorJ6->minAngle)
        valid = false;

    float time = 0;
    if (valid)
        time = MoveJoints(joints);

    return time;
}

float DummyRobot::MoveL(float _x, float _y, float _z, float _a, float _b, float _c)
{
    DOF6Kinematic::Pose6D_t pose6D(_x, _y, _z, _a, _b, _c);
    DOF6Kinematic::IKSolves_t ikSolves{};
    DOF6Kinematic::Joint6D_t lastJoint6D{};

    dof6Solver->SolveIK(pose6D, lastJoint6D, ikSolves);

    bool valid[8];
    int validCnt = 0;
    for (int i = 0; i < 8; i++)
    {
        valid[i] = true;
        if (ikSolves.config[i].a[0] - initPose.a[0] > motorJ1->maxAngle ||
            ikSolves.config[i].a[0] - initPose.a[0] < motorJ1->minAngle)
        {
            valid[i] = false;
            continue;
        }
        if (ikSolves.config[i].a[1] - initPose.a[1] > motorJ2->maxAngle ||
            ikSolves.config[i].a[1] - initPose.a[1] < motorJ2->minAngle)
        {
            valid[i] = false;
            continue;
        }
        if (ikSolves.config[i].a[2] - initPose.a[2] > motorJ3->maxAngle ||
            ikSolves.config[i].a[2] - initPose.a[2] < motorJ3->minAngle)
        {
            valid[i] = false;
            continue;
        }
        if (ikSolves.config[i].a[3] - initPose.a[3] > motorJ4->maxAngle ||
            ikSolves.config[i].a[3] - initPose.a[3] < motorJ4->minAngle)
        {
            valid[i] = false;
            continue;
        }
        if (ikSolves.config[i].a[4] - initPose.a[4] > motorJ5->maxAngle ||
            ikSolves.config[i].a[4] - initPose.a[4] < motorJ5->minAngle)
        {
            valid[i] = false;
            continue;
        }
        if (ikSolves.config[i].a[5] - initPose.a[5] > motorJ6->maxAngle ||
            ikSolves.config[i].a[5] - initPose.a[5] < motorJ6->minAngle)
        {
            valid[i] = false;
            continue;
        }

        if (valid[i]) validCnt++;
    }

    if (validCnt)
    {
        float min = 1000;
        uint8_t index = 0;
        for (int i = 0; i < 8; i++)
        {
            if (valid[i])
            {
                for (int j = 0; j < 6; j++)
                    lastJoint6D.a[j] = ikSolves.config[i].a[j];
                DOF6Kinematic::Joint6D_t tmp = currentJoints - lastJoint6D;
                float maxAngle = AbsMaxOf6(tmp);
                if (maxAngle < min)
                {
                    min = maxAngle;
                    index = i;
                }
            }
        }

        return MoveJ(ikSolves.config[index].a[0], ikSolves.config[index].a[1], ikSolves.config[index].a[2],
                     ikSolves.config[index].a[3], ikSolves.config[index].a[4], ikSolves.config[index].a[5]);
    }

    return 0;
}

void DummyRobot::UpdateJointPos()
{
    motorJ1->UpdateAngle();
    motorJ2->UpdateAngle();
    motorJ3->UpdateAngle();
    motorJ4->UpdateAngle();
    motorJ5->UpdateAngle();
    motorJ6->UpdateAngle();
}

void DummyRobot::SetJointSpeed(float _speed)
{
    jointSpeed = _speed;
}

void DummyRobot::Homing(bool _alreadyCalib)
{
    if (_alreadyCalib)
    {
        MoveJ(0, 0, 90, 0, 0, 0);
    } else
    {
        float _angle = motorJ2->inverseDirection ? -75 : 75;
        float stepMotorCnt = _angle / 360.0f * motorJ2->reduction * CtrlStepMotor::CTRL_CIRCLE_COUNT;
        motorJ2->SetPositionWithTime(stepMotorCnt, 5);

        _angle = motorJ3->inverseDirection ? 100 : -100; //tricky
        stepMotorCnt = _angle / 360.0f * motorJ3->reduction * CtrlStepMotor::CTRL_CIRCLE_COUNT;
        motorJ3->SetPositionWithTime(stepMotorCnt, 5);

        osDelay(6000);
        motorAll->Reboot();
    }
}

void DummyRobot::Resting()
{
    MoveJ(0, -75, 180, 0, 0, 0);
}

void DummyRobot::SetEnable(bool _enable)
{
    motorAll->SetEnable(_enable);
    hand->SetEnable(_enable);
}

DummyHand::DummyHand(CAN_HandleTypeDef* _hcan, uint8_t _id) :
    nodeID(_id), hcan(_hcan)
{
    txHeader =
        {
            .StdId = 0,
            .ExtId = 0,
            .IDE = CAN_ID_STD,
            .RTR = CAN_RTR_DATA,
            .DLC = 8,
            .TransmitGlobalTime = DISABLE
        };
}

void DummyHand::SetAngle(float _angle)
{
    if (_angle > 30)_angle = 30;
    if (_angle < 0)_angle = 0;

    uint8_t mode = 0x02;
    txHeader.StdId = 7 << 7 | mode;

    // Float to Bytes
    auto* b = (unsigned char*) &_angle;
    for (int i = 0; i < 4; i++)
        canBuf[i] = *(b + i);

    CanSendMessage(get_can_ctx(hcan), canBuf, &txHeader);
}

void DummyHand::SetMaxCurrent(float _val)
{
    if (_val > 1)_val = 1;
    if (_val < 0)_val = 0;

    uint8_t mode = 0x01;
    txHeader.StdId = 7 << 7 | mode;

    // Float to Bytes
    auto* b = (unsigned char*) &_val;
    for (int i = 0; i < 4; i++)
        canBuf[i] = *(b + i);

    CanSendMessage(get_can_ctx(hcan), canBuf, &txHeader);
}

void DummyHand::SetEnable(bool _enable)
{
    if (_enable)
        SetMaxCurrent(maxCurrent);
    else
        SetMaxCurrent(0);
}

float DummyHand::GetAngle()
{
    uint8_t mode = 0x33;
    txHeader.StdId = nodeID << 7 | mode;

    CanSendMessage(get_can_ctx(hcan), canBuf, &txHeader);
}
