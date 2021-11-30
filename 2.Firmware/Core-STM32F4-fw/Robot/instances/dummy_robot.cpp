#include <FreeRTOS.h>
#include "communication.hpp"
#include "dummy_robot.h"


inline float AbsMaxOf6(DOF6Kinematic::Joint6D_t _joints)
{
    float max = -1;

    for (float i: _joints.a)
    {
        if (abs(i) > max)
            max = abs(i);
    }

    return max;
}


DummyRobot::DummyRobot(CAN_HandleTypeDef* _hcan) :
    hcan(_hcan)
{
    motorJ[ALL] = new CtrlStepMotor(_hcan, 0, false, 30, -180, 180);
    motorJ[1] = new CtrlStepMotor(_hcan, 1, true, 50, -170, 170);
    motorJ[2] = new CtrlStepMotor(_hcan, 2, false, 30, -73, 90);
    motorJ[3] = new CtrlStepMotor(_hcan, 3, true, 30, 35, 180);
    motorJ[4] = new CtrlStepMotor(_hcan, 4, false, 24, -180, 180);
    motorJ[5] = new CtrlStepMotor(_hcan, 5, true, 30, -120, 120);
    motorJ[6] = new CtrlStepMotor(_hcan, 6, true, 50, -720, 720);
    hand = new DummyHand(_hcan, 7);

    dof6Solver = new DOF6Kinematic(0.109f, 0.035f, 0.146f, 0.115f, 0.052f, 0.072f);
}


DummyRobot::~DummyRobot()
{
    for (int j = 0; j <= 6; j++)
        delete motorJ[j];

    delete hand;
    delete dof6Solver;
}


void DummyRobot::Reboot()
{
    motorJ[ALL]->Reboot();
    osDelay(500); // waiting for all joints done
    HAL_NVIC_SystemReset();
}


float DummyRobot::MoveJoints(DOF6Kinematic::Joint6D_t _joints)
{
    float maxAngle = AbsMaxOf6(_joints - currentJoints);
    float time = maxAngle / jointSpeed;

    jointsStateFlag = 0;

    for (int j = 1; j <= 6; j++)
        motorJ[j]->SetAngleWithTime(_joints.a[j - 1] - initPose.a[j - 1], time);

    return time;
}


float DummyRobot::MoveJ(float _j1, float _j2, float _j3, float _j4, float _j5, float _j6)
{
    float joints[6] = {_j1, _j2, _j3, _j4, _j5, _j6};
    bool valid = true;

    for (int j = 1; j <= 6; j++)
    {
        if (joints[j - 1] > motorJ[j]->angleLimitMax ||
            joints[j - 1] < motorJ[j]->angleLimitMin)
            valid = false;
    }

    float time = 0;
    if (valid)
        time = MoveJoints(DOF6Kinematic::Joint6D_t(
            joints[0], joints[1], joints[2],
            joints[3], joints[4], joints[5]));

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

        for (int j = 1; j <= 6; j++)
        {
            if (ikSolves.config[i].a[j - 1] > motorJ[j]->angleLimitMax ||
                ikSolves.config[i].a[j - 1] < motorJ[j]->angleLimitMin)
            {
                valid[i] = false;
                continue;
            }
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
    } else
        return 0;
}


void DummyRobot::MoveTrajectoryJ(float _j1, float _j2, float _j3, float _j4, float _j5, float _j6)
{

}


void DummyRobot::MoveTrajectoryL(float _x, float _y, float _z, float _a, float _b, float _c)
{

}


void DummyRobot::UpdateJointAngles()
{
    motorJ[ALL]->UpdateAngle();
}


void DummyRobot::UpdateJointAnglesCallback()
{
    for (int i = 1; i <= 6; i++)
    {
        currentJoints.a[i - 1] = motorJ[i]->angle + initPose.a[i - 1];

        if (motorJ[i]->state == CtrlStepMotor::FINISH)
            jointsStateFlag |= (1 << i);
        else
            jointsStateFlag &= ~(1 << i);
    }
}


void DummyRobot::SetJointSpeed(float _speed)
{
    if (_speed < 0)_speed = 0;
    else if (_speed > 100) _speed = 100;

    jointSpeed = _speed;
}


void DummyRobot::SetJointAcceleration(float _acc)
{
    if (_acc < 0)_acc = 0;
    else if (_acc > 1000) _acc = 1000;

    motorJ[ALL]->SetAcceleration(_acc, false);
}


void DummyRobot::CalibrateHomeOffset()
{
    // 1.Manually move joints to L-Pose [precisely]
    // ...
    motorJ[2]->SetCurrentLimit(0.5);
    motorJ[3]->SetCurrentLimit(0.5);
    osDelay(500);

    // 2.Apply Home-Offset the first time
    motorJ[ALL]->ApplyPositionAsHome();
    osDelay(500);

    // 3.Go to Resting-Pose
    initPose = DOF6Kinematic::Joint6D_t(0, 0, 90, 0, 0, 0);
    currentJoints = DOF6Kinematic::Joint6D_t(0, 0, 90, 0, 0, 0);
    Resting();
    while (IsMoving())
        osDelay(10);
    osDelay(500);

    // 4.Apply Home-Offset the second time
    motorJ[ALL]->ApplyPositionAsHome();
    osDelay(500);
    motorJ[2]->SetCurrentLimit(1);
    motorJ[3]->SetCurrentLimit(1);
    osDelay(500);

    Reboot();
}


void DummyRobot::Homing()
{
    float lastSpeed = jointSpeed;
    SetJointSpeed(10);

    MoveJ(0, 0, 90, 0, 0, 0);
    while (IsMoving())
        osDelay(10);

    SetJointSpeed(lastSpeed);
}


void DummyRobot::Resting()
{
    float lastSpeed = jointSpeed;
    SetJointSpeed(10);

    MoveJ(REST_POSE.a[0], REST_POSE.a[1], REST_POSE.a[2],
          REST_POSE.a[3], REST_POSE.a[4], REST_POSE.a[5]);
    while (IsMoving())
        osDelay(10);

    SetJointSpeed(lastSpeed);
}


void DummyRobot::SetEnable(bool _enable)
{
    motorJ[ALL]->SetEnable(_enable);
    hand->SetEnable(_enable);
}


void DummyRobot::UpdateJointPose6D()
{
    dof6Solver->SolveFK(currentJoints, currentPose6D);
    currentPose6D.X *= 1000; // m -> mm
    currentPose6D.Y *= 1000; // m -> mm
    currentPose6D.Z *= 1000; // m -> mm
}


bool DummyRobot::IsMoving()
{
    return jointsStateFlag != 0b1111110;
}


void DummyRobot::SetCommandMode(uint8_t _mode)
{
    commandMode = static_cast<CommandMode>(_mode);
    switch (commandMode)
    {
        case COMMAND_TARGET_POINT_SEQUENTIAL:
            SetJointAcceleration(DEFAULT_JOINT_ACCELERATION);
            break;
        case COMMAND_TARGET_POINT_INTERRUPTABLE:
            SetJointAcceleration(DEFAULT_JOINT_ACCELERATION);
            break;
        case COMMAND_CONTINUES_TRAJECTORY:
            SetJointAcceleration(1000);
            break;
    }
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


uint32_t DummyRobot::CommandHandler::Push(const std::string &_cmd)
{
    osStatus_t status = osMessageQueuePut(commandFifo, _cmd.c_str(), 0U, 0U);
    if (status == osOK)
        return osMessageQueueGetSpace(commandFifo);

    return 0xFF; // failed
}


void DummyRobot::CommandHandler::EmergencyStop()
{
    context->MoveJ(context->currentJoints.a[0], context->currentJoints.a[1], context->currentJoints.a[2],
                   context->currentJoints.a[3], context->currentJoints.a[4], context->currentJoints.a[5]);

    context->isStopped = true;
    ClearFifo();
}


void DummyRobot::CommandHandler::Resume()
{
    context->isStopped = false;
}


std::string DummyRobot::CommandHandler::Pop(uint32_t timeout)
{
    osStatus_t status = osMessageQueueGet(commandFifo, strBuffer, nullptr, timeout);

    return std::string{strBuffer};
}


uint32_t DummyRobot::CommandHandler::GetSpace()
{
    return osMessageQueueGetSpace(commandFifo);
}


uint32_t DummyRobot::CommandHandler::ParseCommand(const std::string &_cmd)
{
    uint8_t argNum;

    switch (context->commandMode)
    {
        case COMMAND_TARGET_POINT_SEQUENTIAL:
            if (_cmd[0] == '>')
            {
                float joints[6];
                float speed;

                argNum = sscanf(_cmd.c_str(), ">%f,%f,%f,%f,%f,%f,%f", joints, joints + 1, joints + 2,
                                joints + 3, joints + 4, joints + 5, &speed);
                if (argNum == 6)
                {
                    context->MoveJ(joints[0], joints[1], joints[2],
                                   joints[3], joints[4], joints[5]);
                } else if (argNum == 7)
                {
                    context->SetJointSpeed(speed);
                    context->MoveJ(joints[0], joints[1], joints[2],
                                   joints[3], joints[4], joints[5]);
                }

                while (context->IsMoving())
                    osDelay(10);

                Respond(*usbStreamOutputPtr, "ok");
            }
            break;

        case COMMAND_TARGET_POINT_INTERRUPTABLE:
            if (_cmd[0] == '>')
            {
                float joints[6];
                float speed;

                argNum = sscanf(_cmd.c_str(), ">%f,%f,%f,%f,%f,%f,%f", joints, joints + 1, joints + 2,
                                joints + 3, joints + 4, joints + 5, &speed);
                if (argNum == 6)
                {
                    context->MoveJ(joints[0], joints[1], joints[2],
                                   joints[3], joints[4], joints[5]);
                } else if (argNum == 7)
                {
                    context->SetJointSpeed(speed);
                    context->MoveJ(joints[0], joints[1], joints[2],
                                   joints[3], joints[4], joints[5]);
                }

                Respond(*usbStreamOutputPtr, "ok");
            }
            break;

        case COMMAND_CONTINUES_TRAJECTORY:
            break;
    }

    return osMessageQueueGetSpace(commandFifo);
}


void DummyRobot::CommandHandler::ClearFifo()
{
    osMessageQueueReset(commandFifo);
}

