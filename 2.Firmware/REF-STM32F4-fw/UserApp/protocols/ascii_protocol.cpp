#include "common_inc.h"

extern DummyRobot dummy;


void OnUsbAsciiCmd(const char* _cmd, size_t _len, StreamSink &_responseChannel)
{
    /*---------------------------- ↓ Add Your CMDs Here ↓ -----------------------------*/
    uint8_t argNum;

    if (_cmd[0] == '#')
    {
        std::string s(_cmd);
        if (s.find("GETJPOS") != std::string::npos)
        {
            dummy.UpdateJointPos();
            osDelay(5);
            Respond(_responseChannel, false, "JNTS %.3f %.3f %.3f %.3f %.3f %.3f",
                    dummy.currentJoints.a[0], dummy.currentJoints.a[1], dummy.currentJoints.a[2],
                    dummy.currentJoints.a[3], dummy.currentJoints.a[4], dummy.currentJoints.a[5]);
        } else
            Respond(_responseChannel, false, "ok");
    } else if (_cmd[0] == '&')
    {
        float pose[6];
        float speed;
        argNum = sscanf(_cmd, "&%f,%f,%f,%f,%f,%f", pose, pose + 1, pose + 2,
                        pose + 3, pose + 4, pose + 5);
        if (argNum == 6)
        {
            float time = dummy.MoveJ(pose[0], pose[1], pose[2],
                                     pose[3], pose[4], pose[5]);

            osDelay(time * 1000); // wait move done
            Respond(_responseChannel, false, "ok");
        }
    } else if (_cmd[0] == '@')
    {
        float pose[6];

        argNum = sscanf(_cmd, "@%f,%f,%f,%f,%f,%f", pose, pose + 1, pose + 2,
                        pose + 3, pose + 4, pose + 5);
        if (argNum == 6)
        {
            float time =  dummy.MoveL(pose[0], pose[1], pose[2],
                        pose[3], pose[4], pose[5]);

            Respond(_responseChannel, false, "ok");
        }
    }
/*---------------------------- ↑ Add Your CMDs Here ↑ -----------------------------*/
}


void OnUartAsciiCmd(const char* _cmd, size_t _len, StreamSink &_responseChannel)
{
    /*---------------------------- ↓ Add Your CMDs Here ↓ -----------------------------*/
    uint8_t argNum;


/*---------------------------- ↑ Add Your CMDs Here ↑ -----------------------------*/
}