#ifndef CTRL_STEP_FW_MOTION_PLANNER_H
#define CTRL_STEP_FW_MOTION_PLANNER_H

#include <cstdint>

class MotionPlanner
{
public:
    MotionPlanner() = default;


    const int32_t CONTROL_FREQUENCY = 20000;                    // Hz
    const int32_t  CONTROL_PERIOD = 1000000 / CONTROL_FREQUENCY; // uS

    struct Config_t
    {
        int32_t encoderHomeOffset;
        int32_t caliCurrent;
        int32_t ratedCurrent;
        int32_t ratedVelocity;
        int32_t ratedVelocityAcc;
        int32_t ratedCurrentAcc;
    };

    class CurrentTracker
    {
    public:
        explicit CurrentTracker(MotionPlanner* _context) :
            context(_context)
        {
        }


        int32_t goCurrent = 0;


        void Init();
        void SetCurrentAcc(int32_t _currentAcc);
        void NewTask(int32_t _realCurrent);
        void CalcSoftGoal(int32_t _goalCurrent);


    private:
        MotionPlanner* context;
        int32_t currentAcc = 0;
        int32_t currentIntegral = 0;
        int32_t trackCurrent = 0;


        void CalcCurrentIntegral(int32_t _current);
    };
    CurrentTracker currentTracker = CurrentTracker(this);

    class VelocityTracker
    {
    public:
        explicit VelocityTracker(MotionPlanner* _context) :
            context(_context)
        {
        }


        int32_t goVelocity = 0;


        void Init();
        void SetVelocityAcc(int32_t _velocityAcc);
        void NewTask(int32_t _realVelocity);
        void CalcSoftGoal(int32_t _goalVelocity);


    private:
        MotionPlanner* context;
        int32_t velocityAcc = 0;
        int32_t velocityIntegral = 0;
        int32_t trackVelocity = 0;


        void CalcVelocityIntegral(int32_t _velocity);
    };
    VelocityTracker velocityTracker = VelocityTracker(this);

    class PositionTracker
    {
    public:
        explicit PositionTracker(MotionPlanner* _context) :
            context(_context)
        {
        }


        int32_t go_location = 0;
        int32_t go_velocity = 0;


        void Init();
        void SetVelocityAcc(int32_t value);
        void NewTask(int32_t real_location, int32_t real_speed);
        void CalcSoftGoal(int32_t _goalPosition);


    private:
        MotionPlanner* context;
        int32_t velocityUpAcc = 0;
        int32_t velocityDownAcc = 0;
        float quickVelocityDownAcc = 0;
        int32_t speedLockingBrake = 0;
        int32_t velocityIntegral = 0;
        int32_t trackVelocity = 0;
        int32_t positionIntegral = 0;
        int32_t trackPosition = 0;


        void CalcVelocityIntegral(int32_t value);
        void CalcPositionIntegral(int32_t value);
    };
    PositionTracker positionTracker = PositionTracker(this);

    class PositionInterpolator
    {
    public:
        explicit PositionInterpolator(MotionPlanner* _context) :
            context(_context)
        {
        }


        int32_t goPosition = 0;
        int32_t goVelocity = 0;


        void Init();
        void NewTask(int32_t _realPosition, int32_t _realVelocity);
        void CalcSoftGoal(int32_t _goalPosition);


    private:
        MotionPlanner* context;
        int32_t recordPosition = 0;
        int32_t recordPositionLast = 0;
        int32_t estPosition = 0;
        int32_t estPositionIntegral = 0;
        int32_t estVelocity = 0;
    };
    PositionInterpolator positionInterpolator = PositionInterpolator(this);

    class TrajectoryTracker
    {
    public:
        explicit TrajectoryTracker(MotionPlanner* _context) :
            context(_context)
        {
        }


        int32_t goPosition = 0;
        int32_t goVelocity = 0;


        void Init(int32_t _updateTimeout);
        void SetSlowDownVelocityAcc(int32_t value);
        void NewTask(int32_t real_location, int32_t real_speed);
        void CalcSoftGoal(int32_t _goalPosition, int32_t _goalVelocity);


    private:
        MotionPlanner* context;
        int32_t velocityDownAcc = 0;
        int32_t dynamicVelocityAcc = 0;
        int32_t updateTime = 0;
        int32_t updateTimeout = 200; // (ms) motion set-points cmd max interval
        bool overtimeFlag = false;
        int32_t recordVelocity = 0;
        int32_t recordPosition = 0;
        int32_t dynamicVelocityAccRemainder = 0;
        int32_t velocityNow = 0;
        int32_t velovityNowRemainder = 0;
        int32_t positionNow = 0;


        void CalcVelocityIntegral(int32_t value);
        void CalcPositionIntegral(int32_t value);
    };
    TrajectoryTracker trajectoryTracker = TrajectoryTracker(this);


    void AttachConfig(Config_t* _config);

private:
    Config_t* config = nullptr;
};


#endif
