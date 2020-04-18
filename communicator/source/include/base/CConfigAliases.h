/***
 * CConfigAliases.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef _C_CONFIG_ALIASES_H_
#define _C_CONFIG_ALIASES_H_

#include <list>
#include <map>
#include <string>
#include <memory>
#include <cassert>

#include <Enum_common.h>

// 'key' assignment of configuration.
#define CONFIG_ALIAS_LIST       "aliases"
#define CONFIG_ALIAS_PVD_TYPE   "provider-type"
#define CONFIG_ALIAS_ADDR       "address"
#define CONFIG_ALIAS_IP         "ip"                // for 'udp' , 'tcp
#define CONFIG_ALIAS_MASK       "mask"              // for 'udp' , 'tcp
#define CONFIG_ALIAS_PORT       "port"              // for 'udp' , 'tcp
#define CONFIG_ALIAS_SVC_ID     "service-id"        // for 'vsomeip'
#define CONFIG_ALIAS_INST_ID    "instance-id"       // for 'vsomeip'


namespace cf_alias {

    typedef enum E_ERROR {
        E_NO_ERROR = 0,
        E_NOT_SUPPORTED_KEY = 1,
        E_NOT_SUPPORTED_PVD_TYPE = 2
    }E_ERROR;

    class IAlias {
    public:
        IAlias(const char* alias_, const char* pvd_type_);

        ~IAlias(void);

        static std::string get_pvd_type(enum_c::ProviderType pvd_type);

    private:
        enum_c::ProviderType cvt_str2pvdtype(std::string pvd_type_str);

    public:
        std::string alias;
        enum_c::ProviderType pvd_type;
    };

    class CAliasTrans : public IAlias {
    public:
        CAliasTrans(const char* alias, const char* pvd_type)
        : IAlias(alias, pvd_type) {
            this->ip.clear();
            this->port_num = 0;
            this->mask = 0;
        }

        ~CAliasTrans(void) {
            ip.clear();
            mask = 0;
            port_num = 0;
        }

    public:
        std::string ip;
        int mask;
        int port_num;
    };

    class CAliasService : public IAlias {
    public:
        CAliasService(const char* alias, const char* pvd_type)
        : IAlias(alias, pvd_type) {
            this->svc_id = 0;
            this->inst_id = 0;
        }

        ~CAliasService(void) {
            svc_id = 0;
            inst_id = 0;
        }

    public:
        int svc_id;
        int inst_id;
    };


    class CConfigAliases {
    public:
        static constexpr char* UDP = "udp";
        static constexpr char* TCP = "tcp";
        static constexpr char* VSOMEIP = "vsomeip";
        using AliasType = std::list<std::shared_ptr<IAlias>>;

    private:
        using AliasMapType = std::map<std::string /* provider_type */, AliasType>;

    public:
        CConfigAliases(const char* config_path);

        ~CConfigAliases(void);

        AliasType& get_aliases(std::string type);

    private:
        CConfigAliases(void) = delete;

        bool init(const std::string config_path);

    private:
        // // valid 'value' assignment of configuration.
        static const char* pvd_types[];

        // provider_type : 'udp' , 'tcp' , 'vsomeip'
        AliasMapType aliases;

        bool f_ready;

    };

}   // cf_alias

#endif // _C_CONFIG_ALIASES_H_