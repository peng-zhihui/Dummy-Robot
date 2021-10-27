
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

        // Add Hierarchy Menus
        make_protocol_object("config",
            // Add Read-Write Variables
                             make_protocol_property("can_node_id", &boardConfig.canNodeId)
        ),

        // Add Functions (must be Member-Functions, so we add a staticFunctions as Helper-Object)
        make_protocol_function("test_function", staticFunctions, &HelperFunctions::TestFunction, "delta"),
        make_protocol_function("save_configuration", staticFunctions,
                               &HelperFunctions::SaveConfigurationHelper),
        make_protocol_function("erase_configuration", staticFunctions,
                               &HelperFunctions::EraseConfigurationHelper),
        make_protocol_function("get_temperature", staticFunctions, &HelperFunctions::GetTemperatureHelper),
        make_protocol_function("reboot", staticFunctions, &HelperFunctions::SystemResetHelper),
        make_protocol_object("robot", dummy.MakeProtocolDefinitions())
    );
}


COMMIT_PROTOCOL
