#ifndef REF_STM32F4_FW_DUMMY_ROBOT_H
#define REF_STM32F4_FW_DUMMY_ROBOT_H

#include "algorithms/kinematic/6dof_kinematic.h"
#include "actuators/ctrl_step/ctrl_step.hpp"

class DummyHand
{
private:
    CAN_HandleTypeDef* hcan;
    uint8_t canBuf[8];
    CAN_TxHeaderTypeDef txHeader;

    float minAngle = 0;
    float maxAngle = 45;

public:
    uint8_t nodeID = 7;
    float maxCurrent = 0.7;

    DummyHand(CAN_HandleTypeDef* _hcan, uint8_t _id);

    void SetAngle(float _angle);
    void SetMaxCurrent(float _val);
    void SetEnable(bool _enable);
    float GetAngle();

    // Communication protocol definitions
    auto MakeProtocolDefinitions()
    {
        return make_protocol_member_list(
            make_protocol_function("set_angle", *this, &DummyHand::SetAngle, "angle"),
            make_protocol_function("set_enable", *this, &DummyHand::SetEnable, "enable"),
            make_protocol_function("set_current_limit", *this, &DummyHand::SetMaxCurrent, "current")
        );
    }
};

class DummyRobot
{
private:
    CAN_HandleTypeDef* hcan;

    float jointSpeed = 20;
    float endpointSpeed;

    DOF6Kinematic* dof6Solver;
public:
    explicit DummyRobot(CAN_HandleTypeDef* _hcan);
    ~DummyRobot();

    DOF6Kinematic::Joint6D_t currentJoints = {0, 0, 90, 0, 0, 0};
    DOF6Kinematic::Joint6D_t initPose = {0, 0, 90, 0, 0, 0};

    CtrlStepMotor* motorAll;
    CtrlStepMotor* motorJ1;
    CtrlStepMotor* motorJ2;
    CtrlStepMotor* motorJ3;
    CtrlStepMotor* motorJ4;
    CtrlStepMotor* motorJ5;
    CtrlStepMotor* motorJ6;
    DummyHand* hand;

    float MoveJoints(DOF6Kinematic::Joint6D_t &_joints);


    // Unit is degree/s
    float MoveJ(float _j1, float _j2, float _j3, float _j4, float _j5, float _j6);
    // Unit is mm/s
    float MoveL(float _x, float _y, float _z, float _a, float _b, float _c);

    void SetJointSpeed(float _speed);

    void UpdateJointPos();

    void Reboot();
    void SetEnable(bool _enable);
    void Homing(bool _alreadyCalib = false);
    void Resting();

    // Communication protocol definitions
    auto MakeProtocolDefinitions()
    {
        return make_protocol_member_list(
            make_protocol_function("update_joint_pos", *this, &DummyRobot::UpdateJointPos),
            make_protocol_function("homing", *this, &DummyRobot::Homing, "already_pose_L"),
            make_protocol_function("resting", *this, &DummyRobot::Resting),
            make_protocol_object("joint_1", motorJ1->MakeProtocolDefinitions()),
            make_protocol_object("joint_2", motorJ2->MakeProtocolDefinitions()),
            make_protocol_object("joint_3", motorJ3->MakeProtocolDefinitions()),
            make_protocol_object("joint_4", motorJ4->MakeProtocolDefinitions()),
            make_protocol_object("joint_5", motorJ5->MakeProtocolDefinitions()),
            make_protocol_object("joint_6", motorJ6->MakeProtocolDefinitions()),
            make_protocol_object("joint_all", motorAll->MakeProtocolDefinitions()),
            make_protocol_object("hand", hand->MakeProtocolDefinitions()),
            make_protocol_function("reboot", *this, &DummyRobot::Reboot),
            make_protocol_function("set_enable", *this, &DummyRobot::SetEnable, "enable"),
            make_protocol_function("move_j", *this, &DummyRobot::MoveJ, "j1", "j2", "j3",
                                   "j4", "j5", "j6"),
            make_protocol_function("move_l", *this, &DummyRobot::MoveL, "x", "y", "z",
                                   "a", "b", "c"),
            make_protocol_function("set_joint_speed", *this, &DummyRobot::SetJointSpeed, "speed")
        );
    }
};

#endif //REF_STM32F4_FW_DUMMY_ROBOT_H
