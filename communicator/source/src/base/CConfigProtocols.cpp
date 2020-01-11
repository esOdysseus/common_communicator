
#include <cassert>
#include <algorithm>

#include <logger.h>
#include <json_manipulator.h>
#include <CConfigProtocols.h>

namespace cf_proto {

static const char* exception_switch(E_ERROR err_num) {
    switch(err_num) {
    case E_ERROR::E_NO_ERROR:
        return "E_NO_ERROR in cf_proto pkg.";
    case E_ERROR::E_OVERFLOW_MAX_ELEMENTS_COUNT:
        return "E_OVERFLOW_MAX_ELEMENTS_COUNT in cf_proto pkg.";
    default:
        return "\'not support error_type\' in payload pkg.";
    }
}
#include <CException.h>

/***********************************
 * Public Function Definition.
 */
CConfigProtocols::CConfigProtocols(void)
: f_ready(false), proto_h(NULL) {
    this->config_full_path.clear();
    this->lib_path.clear();
    this->lib_id.clear();
    this->proto_st.clear();
    // Set dumy-protocol for Empty-protocol config-case.
    this->proto_list = std::make_shared<ProtoList>();
    this->proto_list->push_back(payload::CPayload::Default_Name);
    this->proto_chains_map.clear();
}

CConfigProtocols::CConfigProtocols(std::string config_path)
: f_ready(false), proto_h(NULL) {
    assert( init(config_path) == true );
    assert( dynamic_lib_load(this->lib_id, this->lib_path) == true );
    assert( check_available(this->proto_h, this->proto_list) == true );
    f_ready = true;
}

CConfigProtocols::~CConfigProtocols(void) {
    f_ready = false;
    if ( proto_h ) {
        proto_h->common.close((pt_standard_t*)proto_h);
        proto_h = NULL;
    }
    this->config_full_path.clear();
    this->lib_path.clear();
    this->lib_id.clear();
    this->proto_st.clear();
    this->proto_list->clear();
    this->proto_list.reset();
    this->proto_chains_map.clear();
}

bool CConfigProtocols::is_ready(void) {
    return f_ready;
}

std::shared_ptr<payload::CPayload> CConfigProtocols::create_protocols_chain(void) {
    assert( available_protocols()->size() > 0 );
    LOGD("Called");

    try {
        if( proto_chains_map.size() >= MAX_PROTOCOL_CHAIN_INSTANCES ) {
            throw CException(E_ERROR::E_OVERFLOW_MAX_ELEMENTS_COUNT);
        }
        LOGI("Protocol-Chain instance is filled like [%d/%d].", proto_chains_map.size(), MAX_PROTOCOL_CHAIN_INSTANCES);

        // make protocol-chain instance.
        auto proto_chain = std::make_shared<IProtocolInf::ProtoChainType>();
        std::string chain_name = std::to_string( (unsigned long)(proto_chain.get()) );
        proto_chains_map[chain_name] = proto_chain;

        for (auto itr=available_protocols()->begin(); itr!=available_protocols()->end(); itr++) {
            // append protocol to protocol-chain.
            std::shared_ptr<IProtocolInf> protocol = create_inst(*itr);
            proto_chain->push_back(protocol);
            protocol->set_proto_chain(chain_name, proto_chain);     // register chain to protocol.
        }

        return (*proto_chain->begin());
    }
    catch ( const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }

    return std::make_shared<payload::CPayload>();
}

bool CConfigProtocols::destroy_protocols_chain(std::shared_ptr<payload::CPayload> payload) {
    bool res = false;
    std::string chain_name = payload->get_protocols_chain_name();
    assert(chain_name.empty() == false);
    LOGD("Called");

    try{
        auto itr = proto_chains_map.find(chain_name);
        if (itr != proto_chains_map.end()) {
            // itr->second->clear();   // Clear elements in List.
            itr->second.reset();    // Delete shared_ptr of List.
            proto_chains_map.erase(itr);    // Delete protocol-chain instance.
            res = true;
        }
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return res;    
}

std::shared_ptr<CConfigProtocols::ProtoList> CConfigProtocols::available_protocols(void) {
    return proto_list;
}


/***********************************
 * Private Function Definition.
 */
static bool biuld_protocol_struct(std::string &name, 
                                  protocol_t& pt_dest, 
                                  std::shared_ptr<json_mng::CMjson> &pt_desp,
                                  int proto_index) {
    using json = json_mng::CMjson;
    bool res = false;

    try {
        json_mng::MemberIterator itor;
        auto obj_members = pt_desp->get_member<json_mng::CMjson>(name);
        auto index = obj_members->get_member<int>(CONFIG_PROTO_META_INDEX);
        auto properties = obj_members->get_member<json_mng::CMjson>(CONFIG_PROTO_META_PROPERTY);

        // set meta-members of protocol struct.
        pt_dest.name = name;
        pt_dest.meta.index = (*index.get());
        pt_dest.properties = std::make_shared<PropertyMap>();
        PropertyMap* property_map = pt_dest.properties.get();
        assert(property_map != NULL);
        assert(pt_dest.meta.index == proto_index);
        LOGD("name=%s", pt_dest.name.c_str());
        LOGD("index=%d", pt_dest.meta.index);

        // set properties-member of protocol struct.
        for( itor=properties->begin(); itor!=properties->end(); itor++ ) {
            auto key = json::get_first(itor);
            auto value = json::get_second(itor);
            LOGD("properties key=%s, value=%s", key.c_str(), value->c_str());

            (*property_map)[key] = value->c_str();
        }
        res = true;
    } catch ( const std::exception &e ) {
        LOGERR("%", e.what());
        throw e;
    }

    return res;
}

bool CConfigProtocols::init(std::string &config_file_path) {
    bool res = false;

    try{
        // parse json file.
        Json_DataType json_manager = std::make_shared<json_mng::CMjson>();
        assert( json_manager->parse(config_file_path) == true);
        LOGD("protocol-descriptor Json-file parsing is successful.");

        // set member-variables.
        this->config_full_path = config_file_path;
        this->lib_path = json_manager->get_member(CONFIG_PROTO_LIB_PATH)->c_str();
        this->lib_id = json_manager->get_member(CONFIG_PROTO_ID)->c_str();
        this->proto_h = NULL;
        this->proto_st.clear();
        this->proto_list = std::make_shared<ProtoList>();

        // build protocol-struct.
        int proto_index = 0;
        auto proto_names = json_manager->get_array_member(CONFIG_PROTO_LIST);
        auto objects = json_manager->get_member<json_mng::CMjson>(CONFIG_PROTO_DESP);
        for ( auto itr = proto_names->begin(); itr != proto_names->end(); itr++, proto_index++ ) {
            std::string proto_name = (*itr)->c_str();
            LOGD("element-name=%s", proto_name.c_str());
            assert( biuld_protocol_struct(proto_name, proto_st[proto_name], objects, proto_index) == true );
            this->proto_list->push_back(proto_name);
        }
        res = true;
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return res;
}

bool CConfigProtocols::dynamic_lib_load(std::string id, std::string path) {
    assert(id.empty() == false && path.empty() == false);
    assert(id.length() > 0 && path.length() > 0);
    bool res = false;

    try {
        // dynamic-load protocol-library.
        pt_module_t *p_module = NULL;
        assert( pt_get_module(id.c_str(), path.c_str(), (const pt_module_t **)(&p_module) ) == 0 );
        assert( p_module != NULL );
        assert( p_module->tag == PROTOCOL_MODULE_TAG );

        // load cpp_protocol_lib_t struct from PAL module.
        p_module->methods->open(p_module, NULL, (pt_standard_t **)(&this->proto_h));
        assert( this->proto_h != NULL );
        assert( this->proto_h->common.language == E_LANGUAGE::E_LANGUAGE_CPP );
        res = true;
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }

    return res;
}

bool CConfigProtocols::check_available(CConfigProtocols::ProtoModule *proto_lib, 
                                       std::shared_ptr<CConfigProtocols::ProtoList> &proto_names) {
    assert( proto_lib != NULL);
    bool res = false;

    try {
        std::shared_ptr<ProtoList> lib_proto_names = proto_lib->get_available_protocols();

        for (auto itor = proto_names->begin(); itor != proto_names->end(); itor++) {
            res = false;
            for (auto lib_itor = lib_proto_names->begin(); lib_itor != lib_proto_names->end(); lib_itor++) {
                if ( *lib_itor == *itor ) {
                    res = true;
                    break;
                }
            }
            if(res == false) {
                break;
            }
        }
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        res = false;
        throw e;
    }

    return res;
}

std::shared_ptr<IProtocolInf> CConfigProtocols::create_inst(std::string protocol_name) {
    assert( is_ready() == true );
    std::shared_ptr<IProtocolInf> ret;
    LOGD("Called");

    try{
        if (protocol_name == payload::CPayload::Default_Name) {
            ret = std::make_shared<IProtocolInf>();
        }
        else {
            ret = proto_h->create_instance(protocol_name);
        }
    }
    catch( const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }
    
    LOGD("Quit");
    return ret;
}

std::shared_ptr<IProtocolInf> CConfigProtocols::create_inst(int index_num) {
    assert( is_ready() == true );

    try {
        ProtoList::iterator itor = std::next(proto_list->begin(), index_num);
        return create_inst(*itor);
    }
    catch( const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }
}

}   // namespace cf_proto
