
#include "common_inc.h"

/*----------------- 1.Add Your Extern Variables Here (Optional) ------------------*/
extern DummyRobot dummy;

class HelperFunctions
{
public:
    /*--------------- 2.Add Your Helper Functions Helper Here (optional) ----------------*/
    int32_t TestFunction(int32_t delta)
    {
        static int cnt = 0;
        return cnt += delta;
    }

    void SaveConfigurationHelper()
    {}

    void EraseConfigurationHelper()
    {}

    float GetTemperatureHelper()
    { return AdcGetChipTemperature(); }

    void SystemResetHelper()
    { NVIC_SystemReset(); }

} staticFunctions;


// Define options that intractable with "reftool".
static inline auto MakeObjTree()
{
    /*--------------- 3.Add Your Protocol Variables & Functions Here ----------------*/
    return make_protocol_member_list(
        // Add Read-Only Variables
        make_protocol_ro_property("serial_number", &serialNumber),
        make_protocol_object("robot", dummy.MakeProtocolDefinitions())
    );
}


COMMIT_PROTOCOL
