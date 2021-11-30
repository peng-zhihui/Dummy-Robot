#ifndef REF_STM32F4_FW_DUMMY_ROBOT_H
#define REF_STM32F4_FW_DUMMY_ROBOT_H

#include "algorithms/kinematic/6dof_kinematic.h"
#include "actuators/ctrl_step/ctrl_step.hpp"

#define ALL 0

/*
  |   PARAMS   | `current_limit` | `acceleration` | `dce_kp` | `dce_kv` | `dce_ki` | `dce_kd` |
  | ---------- | --------------- | -------------- | -------- | -------- | -------- | -------- |
  | **Joint1** | 1               | 25             | 1000     | 80       | 300      | 250      |
  | **Joint2** | 1.5             | 25             | 1000     | 250      | 300      | 200      |
  | **Joint3** | 1               | 25             | 1000     | 350      | 500      | 250      |
  | **Joint4** | 1.5             | 25             | 1000     | 350      | 500      | 250      |
  | **Joint5** | 1.5             | 25             | 1000     | 350      | 500      | 250      |
  | **Joint6** | 1.5             | 25             | 1000     | 350      | 500      | 250      |
 */


class DummyHand
{
public:
    uint8_t nodeID = 7;
    float maxCurrent = 0.7;


    DummyHand(CAN_HandleTypeDef* _hcan, uint8_t _id);


    void SetAngle(float _angle);
    void SetMaxCurrent(float _val);
    void SetEnable(bool _enable);


    // Communication protocol definitions
    auto MakeProtocolDefinitions()
    {
        return make_protocol_member_list(
            make_protocol_function("set_angle", *this, &DummyHand::SetAngle, "angle"),
            make_protocol_function("set_enable", *this, &DummyHand::SetEnable, "enable"),
            make_protocol_function("set_current_limit", *this, &DummyHand::SetMaxCurrent, "current")
        );
    }


private:
    CAN_HandleTypeDef* hcan;
    uint8_t canBuf[8];
    CAN_TxHeaderTypeDef txHeader;
    float minAngle = 0;
    float maxAngle = 45;
};


class DummyRobot
{
public:
    explicit DummyRobot(CAN_HandleTypeDef* _hcan);
    ~DummyRobot();


    enum CommandMode
    {
        COMMAND_TARGET_POINT_SEQUENTIAL = 1,
        COMMAND_TARGET_POINT_INTERRUPTABLE,
        COMMAND_CONTINUES_TRAJECTORY
    };

    // This is the pose when power on.
    const DOF6Kinematic::Joint6D_t REST_POSE = {0, -73, 180, 0, 0, 0};
    const float DEFAULT_JOINT_SPEED = 30;
    const float DEFAULT_JOINT_ACCELERATION = 50;

    DOF6Kinematic::Joint6D_t currentJoints = REST_POSE;
    DOF6Kinematic::Joint6D_t initPose = REST_POSE;
    DOF6Kinematic::Pose6D_t currentPose6D = {};
    volatile uint8_t jointsStateFlag = 0b00000000;
    CommandMode commandMode = COMMAND_TARGET_POINT_SEQUENTIAL;
    bool isStopped = false;
    CtrlStepMotor* motorJ[7] = {nullptr};
    DummyHand* hand = {nullptr};


    float MoveJ(float _j1, float _j2, float _j3, float _j4, float _j5, float _j6);
    float MoveL(float _x, float _y, float _z, float _a, float _b, float _c);
    void MoveTrajectoryJ(float _j1, float _j2, float _j3, float _j4, float _j5, float _j6);
    void MoveTrajectoryL(float _x, float _y, float _z, float _a, float _b, float _c);
    void SetJointSpeed(float _speed);
    void SetJointAcceleration(float _acc);
    void UpdateJointAngles();
    void UpdateJointAnglesCallback();
    void UpdateJointPose6D();
    void Reboot();
    void SetEnable(bool _enable);
    void CalibrateHomeOffset();
    void Homing();
    void Resting();
    bool IsMoving();
    void SetCommandMode(uint8_t _mode);


    // Communication protocol definitions
    auto MakeProtocolDefinitions()
    {
        return make_protocol_member_list(
            make_protocol_function("calibrate_home_offset", *this, &DummyRobot::CalibrateHomeOffset),
            make_protocol_function("homing", *this, &DummyRobot::Homing),
            make_protocol_function("resting", *this, &DummyRobot::Resting),
            make_protocol_object("joint_1", motorJ[1]->MakeProtocolDefinitions()),
            make_protocol_object("joint_2", motorJ[2]->MakeProtocolDefinitions()),
            make_protocol_object("joint_3", motorJ[3]->MakeProtocolDefinitions()),
            make_protocol_object("joint_4", motorJ[4]->MakeProtocolDefinitions()),
            make_protocol_object("joint_5", motorJ[5]->MakeProtocolDefinitions()),
            make_protocol_object("joint_6", motorJ[6]->MakeProtocolDefinitions()),
            make_protocol_object("joint_all", motorJ[ALL]->MakeProtocolDefinitions()),
            make_protocol_object("hand", hand->MakeProtocolDefinitions()),
            make_protocol_function("reboot", *this, &DummyRobot::Reboot),
            make_protocol_function("set_enable", *this, &DummyRobot::SetEnable, "enable"),
            make_protocol_function("move_j", *this, &DummyRobot::MoveJ, "j1", "j2", "j3", "j4", "j5", "j6"),
            make_protocol_function("move_l", *this, &DummyRobot::MoveL, "x", "y", "z", "a", "b", "c"),
            make_protocol_function("set_joint_speed", *this, &DummyRobot::SetJointSpeed, "speed"),
            make_protocol_function("set_joint_acc", *this, &DummyRobot::SetJointAcceleration, "acc"),
            make_protocol_function("set_command_mode", *this, &DummyRobot::SetCommandMode, "mode")

        );
    }


    class CommandHandler
    {
    public:
        explicit CommandHandler(DummyRobot* _context) : context(_context)
        {
            commandFifo = osMessageQueueNew(16, 64, nullptr);
            commandLifo = osMessageQueueNew(1, 64, nullptr);
        }

        uint32_t Push(const std::string &_cmd);
        std::string Pop(uint32_t timeout);
        uint32_t ParseCommand(const std::string &_cmd);
        uint32_t GetSpace();
        void ClearFifo();
        void EmergencyStop();
        void Resume();


    private:
        DummyRobot* context;
        osMessageQueueId_t commandFifo;
        osMessageQueueId_t commandLifo;
        char strBuffer[64]{};
    };
    CommandHandler commandHandler = CommandHandler(this);


private:
    CAN_HandleTypeDef* hcan;
    float jointSpeed = DEFAULT_JOINT_SPEED;
    DOF6Kinematic* dof6Solver;


    float MoveJoints(DOF6Kinematic::Joint6D_t _joints);
};


#endif //REF_STM32F4_FW_DUMMY_ROBOT_H
