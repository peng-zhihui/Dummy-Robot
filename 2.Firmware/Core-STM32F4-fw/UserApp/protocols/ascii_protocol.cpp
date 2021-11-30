#include "common_inc.h"

extern DummyRobot dummy;


void OnUsbAsciiCmd(const char* _cmd, size_t _len, StreamSink &_responseChannel)
{
    /*---------------------------- ↓ Add Your CMDs Here ↓ -----------------------------*/
    if (_cmd[0] == '!' || dummy.isStopped)
    {
        std::string s(_cmd);
        if (s.find("STOP") != std::string::npos)
        {
            dummy.commandHandler.EmergencyStop();
            Respond(_responseChannel, "!!!Stopped!!!");
        } else if (s.find("RESUME") != std::string::npos)
        {
            dummy.commandHandler.Resume();
            Respond(_responseChannel, "Resumed");
        }
    } else if (_cmd[0] == '#')
    {
        std::string s(_cmd);
        if (s.find("GETJPOS") != std::string::npos)
        {
            Respond(*usbStreamOutputPtr, "JNTS %.2f %.2f %.2f %.2f %.2f %.2f",
                    dummy.currentJoints.a[0], dummy.currentJoints.a[1],
                    dummy.currentJoints.a[2], dummy.currentJoints.a[3],
                    dummy.currentJoints.a[4], dummy.currentJoints.a[5]);
        } else if (s.find("GETLPOS") != std::string::npos)
        {
            dummy.UpdateJointPose6D();
            Respond(*usbStreamOutputPtr, "POSE %.2f %.2f %.2f %.2f %.2f %.2f",
                    dummy.currentPose6D.X, dummy.currentPose6D.Y,
                    dummy.currentPose6D.Z, dummy.currentPose6D.A,
                    dummy.currentPose6D.B, dummy.currentPose6D.C);
        } else if (s.find("CMDMODE") != std::string::npos)
        {
            int mode;
            sscanf(_cmd, "CMDMODE %d", &mode);
            dummy.SetCommandMode(mode);
            Respond(*usbStreamOutputPtr, "Set command mode to [%d]", mode);
        } else
            Respond(*usbStreamOutputPtr, "ok");
    } else if (_cmd[0] == '>' || _cmd[0] == '@')
    {
        uint32_t freeSize = dummy.commandHandler.Push(_cmd);
        Respond(_responseChannel, "%d", freeSize);
    }

/*---------------------------- ↑ Add Your CMDs Here ↑ -----------------------------*/
}


void OnUart4AsciiCmd(const char* _cmd, size_t _len, StreamSink &_responseChannel)
{
    /*---------------------------- ↓ Add Your CMDs Here ↓ -----------------------------*/
    uint8_t argNum;

    if (_cmd[0] == '#')
    {
        std::string s(_cmd);
        if (s.find("GETJPOS") != std::string::npos)
        {
            Respond(_responseChannel, "JNTS %.2f %.2f %.2f %.2f %.2f %.2f",
                    dummy.currentJoints.a[0], dummy.currentJoints.a[1], dummy.currentJoints.a[2],
                    dummy.currentJoints.a[3], dummy.currentJoints.a[4], dummy.currentJoints.a[5]);
        } else
            Respond(_responseChannel, "ok");
    } else if (_cmd[0] == '>')
    {
        float joints[6];
        float speed;

        argNum = sscanf(_cmd, ">%f,%f,%f,%f,%f,%f,%f", joints, joints + 1, joints + 2,
                        joints + 3, joints + 4, joints + 5, &speed);
        if (argNum == 6)
        {
            dummy.MoveJ(joints[0], joints[1], joints[2],
                        joints[3], joints[4], joints[5]);
        } else if (argNum == 7)
        {
            dummy.SetJointSpeed(speed);
            dummy.MoveJ(joints[0], joints[1], joints[2],
                        joints[3], joints[4], joints[5]);
        }

        while (dummy.IsMoving())
            osDelay(10);
        Respond(_responseChannel, "ok");

    } else if (_cmd[0] == '@')
    {
        float pose[6];
        float speed;

        argNum = sscanf(_cmd, "@%f,%f,%f,%f,%f,%f,%f", pose, pose + 1, pose + 2,
                        pose + 3, pose + 4, pose + 5, &speed);
        if (argNum == 6)
        {
            dummy.MoveL(pose[0], pose[1], pose[2],
                        pose[3], pose[4], pose[5]);
        } else if (argNum == 7)
        {
            dummy.SetJointSpeed(speed);
            dummy.MoveL(pose[0], pose[1], pose[2],
                        pose[3], pose[4], pose[5]);
        }

        while (dummy.IsMoving())
            osDelay(10);
        Respond(_responseChannel, "ok");
    }
/*---------------------------- ↑ Add Your CMDs Here ↑ -----------------------------*/
}


void OnUart5AsciiCmd(const char* _cmd, size_t _len, StreamSink &_responseChannel)
{
    /*---------------------------- ↓ Add Your CMDs Here ↓ -----------------------------*/

/*---------------------------- ↑ Add Your CMDs Here ↑ -----------------------------*/
}