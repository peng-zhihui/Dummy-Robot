
#include "common_inc.h"

/*----------------- 1.Add Your Extern Variables Here (Optional) ------------------*/
extern DummyRobot dummy;

class HelperFunctions
{
public:
    /*--------------- 2.Add Your Helper Functions Helper Here (optional) ----------------*/
    float GetTemperatureHelper()
    { return AdcGetChipTemperature(); }

} staticFunctions;


// Define options that intractable with "reftool".
static inline auto MakeObjTree()
{
    /*--------------- 3.Add Your Protocol Variables & Functions Here ----------------*/
    return make_protocol_member_list(
        // Add Read-Only Variables
        make_protocol_ro_property("serial_number", &serialNumber),
        make_protocol_function("get_temperature", staticFunctions, &HelperFunctions::GetTemperatureHelper),
        make_protocol_object("robot", dummy.MakeProtocolDefinitions())
    );
}


COMMIT_PROTOCOL
