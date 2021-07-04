/***
 * CConfigProtocols.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef _CONFIG_PROTOCOL_H_
#define _CONFIG_PROTOCOL_H_

#include <map>
#include <memory>
#include <string>

#include <pal/protocol.h>
#include <IProtocolInf.h>

// for Protocol-Description.
#define CONFIG_PROTO_LIB_PATH       "protocol-libpath"
#define CONFIG_PROTO_ID             "protocol-id"
#define CONFIG_PROTO_LIST           "protocol-list"
#define CONFIG_PROTO_DESP           "protocol-desp"
#define CONFIG_PROTO_META_INDEX     "index"
#define CONFIG_PROTO_META_PROPERTY  "properties"

namespace cf_proto {

    typedef enum E_ERROR {
        E_NO_ERROR = 0,
        E_OVERFLOW_MAX_ELEMENTS_COUNT = 1
    }E_ERROR;

    using PropertyMap = std::map<std::string, std::string>;

    typedef struct protocol_meta_t {
        int index;
    } protocol_meta_t;


    typedef struct protocol_t {
        std::string name;
        protocol_meta_t meta;
        std::shared_ptr<PropertyMap> properties;
    } protocol_t;


    /***********
     * PAL structure for c++
     */
    typedef struct cpp_protocol_lib_t {
        // protocol-context API
        struct pt_standard_t common;

        /** Get available protocol_contexts */
        std::shared_ptr<std::list<std::string>> (*get_available_protocols)(void);
        /** Create instance of protocol correspond with protocol-name. */
        std::shared_ptr<IProtocolInf> (*create_instance)(std::string protocol_name);
    } cpp_protocol_lib_t;


    class CConfigProtocols {
    public:
        using ProtoModule = cpp_protocol_lib_t;
        using ProtoList = std::list<std::string>;

    public:
        CConfigProtocols(const char* config_path);

        ~CConfigProtocols(void);

        bool is_ready(void);

        std::shared_ptr<payload::CPayload> create_protocols_chain(void);

        bool destroy_protocols_chain(std::shared_ptr<payload::CPayload> payload);

        std::shared_ptr<ProtoList>& available_protocols(void);

    private:
        CConfigProtocols(void) = delete;

        bool init(const std::string config_file_path);

        bool dynamic_lib_load(std::string id, std::string path);

        bool check_available(ProtoModule *proto_lib, std::shared_ptr<ProtoList> &proto_names);

        std::shared_ptr<IProtocolInf> create_inst(std::string protocol_name);

        std::shared_ptr<IProtocolInf> create_inst(int index_num);

    private:
        bool f_ready;

        /** It's Full-Path of config/desp_protocol.json file. */
        std::string config_full_path;

        /** It indicate library-full-path for member-variable 'proto_h'. */
        std::string lib_path;

        /** It indicate library-ID for member-variable 'proto_h'. */
        std::string lib_id;

        /** It indicate library-handler for manipulating of protocol-instance. */
        ProtoModule *proto_h;

        /** It's loaded from config/desp_protocol.json file. */
        std::map<std::string, protocol_t> proto_st;

        /** It's indicate available protocol-name list. */
        std::shared_ptr<ProtoList> proto_list;

        /** It's mapper for management of available instances as protocols-chain. */
        std::map<std::string, std::shared_ptr<IProtocolInf::ProtoChainType>> proto_chains_map;

        static constexpr unsigned int MAX_PROTOCOL_CHAIN_INSTANCES = 2048;
    };

}
#endif // _CONFIG_PROTOCOL_H_
