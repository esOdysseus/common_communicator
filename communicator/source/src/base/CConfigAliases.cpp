#include <cassert>

#include <logger.h>
#include <CConfigAliases.h>
#include <json_manipulator.h>

namespace cf_alias {

const char* CConfigAliases::pvd_types[] = { CConfigAliases::UDP, 
                                            CConfigAliases::TCP, 
                                            CConfigAliases::VSOMEIP, 
                                            NULL};

static std::shared_ptr<IAlias> make_alias(std::string alias, std::shared_ptr<json_mng::CMjson> obj_value);
static std::shared_ptr<CAliasTrans> make_alias_trans(std::string alias, std::string pvd_type, 
                                                     std::shared_ptr<json_mng::CMjson> &obj_addr);
static std::shared_ptr<CAliasService> make_alias_service(std::string alias, std::string pvd_type, 
                                                         std::shared_ptr<json_mng::CMjson> &obj_addr);

static const char* exception_switch(E_ERROR err_num) {
    switch(err_num) {
    case E_ERROR::E_NO_ERROR:
        return "E_NO_ERROR in cf_alias pkg.";
    case E_ERROR::E_NOT_SUPPORTED_KEY:
        return "E_NOT_SUPPORTED_KEY in cf_alias pkg.";
    case E_ERROR::E_NOT_SUPPORTED_PVD_TYPE:
        return "E_NOT_SUPPORTED_PVD_TYPE in cf_alias pkg.";
    default:
        return "\'not support error_type\' in cf_alias pkg.";
    }
}
#include <CException.h>

/***************************************
 * Definition of Public Function.
 */
CConfigAliases::CConfigAliases(const char* config_path)
: f_ready(false) {
    try {
        aliases.clear();
        for( int i=0; pvd_types[i] != NULL; i++ ) {
            LOGD("List initialize.(name=%s)", pvd_types[i]);
            aliases[pvd_types[i]].clear();
        }

        if ( config_path != NULL ) {
            LOGD("Append context of ConfigAliases json-file.");
            assert( init(config_path) == true );
        }
        f_ready = true;
    }
    catch ( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

CConfigAliases::~CConfigAliases(void) {
    f_ready = false;
    aliases.clear();
}

CConfigAliases::AliasType& CConfigAliases::get_aliases(std::string type) {
    AliasMapType::iterator itr;
    assert( f_ready == true );
    assert( type.empty() == false );

    try {
        if ( (itr = aliases.find(type)) != aliases.end() ) {
            return aliases[type];
        }
        else {
            LOGW("Not supported pvd_type=%s", type.c_str());
            throw CException(E_ERROR::E_NOT_SUPPORTED_PVD_TYPE);
        }
    }
    catch ( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

bool CConfigAliases::init(const std::string config_path) {
    bool res = false;

    try{
        // parse json file.
        Json_DataType json_manager = std::make_shared<json_mng::CMjson>();
        assert( json_manager->parse(config_path) == true);
        LOGD("aliases-descriptor Json-file parsing is successful.");

        // build alias-struct.
        auto objects = json_manager->get_member<json_mng::CMjson>(CONFIG_ALIAS_LIST);
        for ( auto itr = objects->begin(); itr != objects->end(); itr++ ) {
            std::string alias = json_mng::CMjson::get_first(itr);
            auto obj_value = json_mng::CMjson::get_second<json_mng::CMjson>(itr);
            
            LOGD("===");
            LOGD("= alias-name=%s", alias.c_str());
            std::shared_ptr<IAlias> new_alias = make_alias(alias, obj_value);
            aliases[new_alias->pvd_type].push_back(new_alias);
        }
        res = true;
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        res = false;
        throw e;
    }

    return res;
}


/***************************************************
 * Private Function Definition.
 ***************************************************/
static std::shared_ptr<IAlias> make_alias(std::string alias, std::shared_ptr<json_mng::CMjson> obj_value) {
    std::shared_ptr<IAlias> res;
    assert(alias.empty() == false);
    assert(obj_value.get() != NULL);

    try {
        std::string pvd_type = obj_value->get_member(CONFIG_ALIAS_PVD_TYPE)->c_str();
        auto addr = obj_value->get_member<json_mng::CMjson>(CONFIG_ALIAS_ADDR);

        LOGD("= pvd_type=%s", pvd_type.c_str());
        if ( pvd_type == CConfigAliases::UDP || pvd_type == CConfigAliases::TCP ) {
            res = make_alias_trans(alias, pvd_type, addr);
        }
        else if ( pvd_type == CConfigAliases::VSOMEIP ) {
            res = make_alias_service(alias, pvd_type, addr);
        }
        else {
            throw CException(E_ERROR::E_NOT_SUPPORTED_PVD_TYPE);
        }
        LOGD("===");
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return res;
}

static std::shared_ptr<CAliasTrans> make_alias_trans(std::string alias, 
                                                     std::string pvd_type, 
                                                     std::shared_ptr<json_mng::CMjson> &obj_addr) {
    assert(alias.empty() == false);
    assert(pvd_type.empty() == false);
    assert(obj_addr.get() != NULL);
    std::shared_ptr<CAliasTrans> res = std::make_shared<CAliasTrans>(alias.c_str(), pvd_type.c_str());

    try {
        // extract 'address' part.
        for ( auto itr = obj_addr->begin(); itr != obj_addr->end(); itr++ ) {
            std::string key = json_mng::CMjson::get_first(itr);
            assert( key.empty() == false );
            LOGD("= Key=%s", key.c_str());

            if ( key == CONFIG_ALIAS_IP ) {
                res->ip = json_mng::CMjson::get_second(itr)->c_str();
                LOGD("= Value=%s", res->ip.c_str());
            }
            else if ( key == CONFIG_ALIAS_PORT ) {
                res->port_num = *(json_mng::CMjson::get_second<int>(itr).get());
                LOGD("= Value=%d", res->port_num);
            }
            else if ( key == CONFIG_ALIAS_MASK ) {
                res->mask = *(json_mng::CMjson::get_second<int>(itr).get());
                LOGD("= Value=%d", res->mask);
            }
            else {
                throw CException(E_ERROR::E_NOT_SUPPORTED_KEY);
            }
        }
    }
    catch ( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return res;
}

static std::shared_ptr<CAliasService> make_alias_service(std::string alias, 
                                                         std::string pvd_type, 
                                                         std::shared_ptr<json_mng::CMjson> &obj_addr) {
    assert(alias.empty() == false);
    assert(pvd_type.empty() == false);
    assert(obj_addr.get() != NULL);
    std::shared_ptr<CAliasService> res = std::make_shared<CAliasService>(alias.c_str(), pvd_type.c_str());

    try {
        // extract 'address' part.
        for ( auto itr = obj_addr->begin(); itr != obj_addr->end(); itr++ ) {
            std::string key = json_mng::CMjson::get_first(itr);
            assert( key.empty() == false );
            LOGD("= Key=%s", key.c_str());

            if ( key == CONFIG_ALIAS_SVC_ID ) {
                res->svc_id = *(json_mng::CMjson::get_second<int>(itr).get());
                LOGD("= Value=%d", res->svc_id);
            }
            else if ( key == CONFIG_ALIAS_INST_ID ) {
                res->inst_id = *(json_mng::CMjson::get_second<int>(itr).get());
                LOGD("= Value=%d", res->inst_id);
            }
            else {
                throw CException(E_ERROR::E_NOT_SUPPORTED_KEY);
            }
        }
    }
    catch ( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return res;
}



}   // namespace cf_alias
