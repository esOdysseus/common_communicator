#ifndef ENUM_COMMON_H_
#define ENUM_COMMON_H_

namespace enum_c
{
    typedef enum ProviderType {
        E_PVDT_NOT_DEFINE = 0,
        E_PVDT_TRANS_TCP = 1,
        E_PVDT_TRANS_UDP = 2,
        E_PVDT_TRANS_UDS = 3,
        E_PVDT_SERVICE_VSOMEIP = 4,
        E_PVDT_RSC_IOTIVITY = 5
    } ProviderType;

    enum class ProviderMode {
        E_PVDM_NONE = 0,
        E_PVDM_SERVER = 1,
        E_PVDM_CLIENT = 2,
        E_PVDM_BOTH = 3
    };
}


#endif // ENUM_COMMON_H_