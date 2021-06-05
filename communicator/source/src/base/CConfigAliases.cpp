/***
 * CConfigAliases.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <cassert>
#include <functional>

#include <logger.h>
#include <CConfigAliases.h>

// 'key' assignment of JSON-configuration.
#define CONFIG_ALIAS_LIST       "aliases"
#define CONFIG_ALIAS_PROPERTY   "properties"
#define CONFIG_ALIAS_PROP_TYPE  "type"              // for properties.
#define CONFIG_ALIAS_PROP_NAME  "name"              // for properties.
#define CONFIG_ALIAS_PROP_WHERE "where"             // for properties.
#define CONFIG_ALIAS_APP_LIST   "app"
#define CONFIG_ALIAS_PVD_LIST   "svc-pvd"
#define CONFIG_ALIAS_PVD_TYPE   "provider-type"
#define CONFIG_ALIAS_ADDR       "address"
#define CONFIG_ALIAS_IP         "ip"                // for 'udp' , 'tcp' of address
#define CONFIG_ALIAS_MASK       "mask"              // for 'udp' , 'tcp' of address
#define CONFIG_ALIAS_PORT       "port"              // for 'udp' , 'tcp' of address
#define CONFIG_ALIAS_SVC_ID     "service-id"        // for 'vsomeip' of address
#define CONFIG_ALIAS_INST_ID    "instance-id"       // for 'vsomeip' of address
#define CONFIG_ALIAS_FUNCTION   "functions"
#define CONFIG_ALIAS_REQ_RESP   "reqresp"
#define CONFIG_ALIAS_REQ_NO_RESP "req_noresp"
#define CONFIG_ALIAS_PUB_SUB    "pubsub"
#define CONFIG_ALIAS_FUNC_ID    "id"                // for reqresp , req_noresp , pubsub
#define CONFIG_ALIAS_FUNC_GID   "grp-id"            // for pubsub


namespace cf_alias {

static const char* exception_switch(E_ERROR err_num) {
    switch(err_num) {
    case E_ERROR::E_NO_ERROR:
        return "E_NO_ERROR in cf_alias pkg.";
    case E_ERROR::E_NOT_SUPPORTED_KEY:
        return "E_NOT_SUPPORTED_KEY in cf_alias pkg.";
    case E_ERROR::E_NOT_SUPPORTED_PVD_TYPE:
        return "E_NOT_SUPPORTED_PVD_TYPE in cf_alias pkg.";
    case E_ERROR::E_NOT_SUPPORTED_APP_TYPE:
        return "E_NOT_SUPPORTED_APP_TYPE in cf_alias pkg.";
    case E_ERROR::E_NOT_SUPPORTED_APP_PATH:
        return "E_NOT_SUPPORTED_APP_PATH in cf_alias pkg.";
    default:
        return "\'not support error_type\' in cf_alias pkg.";
    }
}
#include <CException.h>


constexpr const char* CConfigAliases::SINGLE;
constexpr const char* CConfigAliases::MULTIPLE;
constexpr const char* CConfigAliases::SELF;

const char* CConfigAliases::_ma_pvd_types_[] = { 
    IAliasPVD::UDP, 
    IAliasPVD::UDP_UDS, 
    IAliasPVD::TCP, 
    IAliasPVD::TCP_UDS, 
    IAliasPVD::VSOMEIP, 
    NULL
};


/***************************************
 * Definition of Public Function.
 */
CConfigAliases::CConfigAliases(const char* config_path)
: _m_f_ready_(false) {
    try {
        _mm_pvds_.clear();
        for( int i=0; _ma_pvd_types_[i] != NULL; i++ ) {
            LOGD("PVD-List initialize.(name=%s)", _ma_pvd_types_[i]);
            _mm_pvds_[_ma_pvd_types_[i]].clear();
        }

        _mm_pvds_map_.clear();
        _mm_rscs_.clear();

        if ( config_path != NULL ) {
            LOGD("Append context of ConfigAliases json-file.");
            assert( init(config_path) == true );
        }
        _m_f_ready_ = true;
    }
    catch ( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

CConfigAliases::~CConfigAliases(void) {
    _m_f_ready_ = false;
    _mm_pvds_.clear();
    _mm_pvds_map_.clear();
    _mm_rscs_.clear();
}

CConfigAliases::PVDListType& CConfigAliases::get_providers(std::string pvd_type) {
    PVDMapType::iterator itr;
    assert( _m_f_ready_ == true );
    assert( pvd_type.empty() == false );

    try {
        itr = _mm_pvds_.find(pvd_type);
        if ( itr == _mm_pvds_.end() ) {
            LOGW("Not supported pvd_type=%s", pvd_type.c_str());
            throw CException(E_ERROR::E_NOT_SUPPORTED_PVD_TYPE);
        }

        return itr->second;
    }
    catch ( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

CConfigAliases::PVDListType& CConfigAliases::get_providers(std::string app_path, std::string pvd_type) {
    PVDMapType::iterator itr;
    assert( _m_f_ready_ == true );
    assert( pvd_type.empty() == false );

    try {
        auto itr_app = _mm_pvds_map_.find(app_path);
        if( itr_app == _mm_pvds_map_.end() ) {
            LOGW("Not exist APP-path(%s)", app_path.data());
            throw CException(E_ERROR::E_NOT_SUPPORTED_APP_PATH);
        }

        itr = itr_app->second.find(pvd_type);
        if( itr == itr_app->second.end() ) {
            LOGW("Not supported pvd_type=%s", pvd_type.data());
            throw CException(E_ERROR::E_NOT_SUPPORTED_PVD_TYPE);
        }

        return itr->second;
    }
    catch ( const CException &e ) {
        LOGW("%s", e.what());
        throw e;
    }
    catch ( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

CConfigAliases::PVDListType& CConfigAliases::get_providers(std::string app_path, enum_c::ProviderType pvd_type) {
    return get_providers( app_path, IAliasPVD::convert(pvd_type) );
}

std::shared_ptr<IAliasPVD> CConfigAliases::get_provider(std::string app_path, std::string pvd_id) {
    std::shared_ptr<IAliasPVD> result;

    try {
        for( int i=0; _ma_pvd_types_[i] != NULL; i++ ) {
            result = get_provider( app_path, pvd_id, _ma_pvd_types_[i] );
            
            if( result.get() != NULL ) {
                LOGI("Found provider.(%s/%s)", app_path.data(), pvd_id.data());
                break;
            }
        }
    }
    catch ( const CException &e ) {
        LOGW("%s", e.what());
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return result;
}

std::shared_ptr<IAliasPVD> CConfigAliases::get_provider(std::string app_path, std::string pvd_id, std::string pvd_type) {
    std::shared_ptr<IAliasPVD> result;

    try {
        PVDListType& pvds_list = get_providers(std::move(app_path), std::move(pvd_type));

        for( auto itr = pvds_list.begin(); itr != pvds_list.end(); itr++ ) {
            if( (*itr)->name() == pvd_id ) {
                result = *itr;
                break;
            }
        }
    }
    catch ( const CException &e ) {
        LOGW("%s", e.what());
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return result;
}

std::shared_ptr<IAliasPVD> CConfigAliases::get_provider(std::string app_path, std::string pvd_id, enum_c::ProviderType pvd_type) {
    return get_provider( app_path, pvd_id, IAliasPVD::convert(pvd_type) );
}

std::shared_ptr<IAliasPVD> CConfigAliases::create_provider(std::string app_path, std::string pvd_id, enum_c::ProviderType pvd_type) {
    try {
        switch( pvd_type ) {
        case enum_c::ProviderType::E_PVDT_TRANS_TCP:
        case enum_c::ProviderType::E_PVDT_TRANS_UDP:
        case enum_c::ProviderType::E_PVDT_TRANS_UDS_TCP:
        case enum_c::ProviderType::E_PVDT_TRANS_UDS_UDP:
            return std::make_shared<CAliasTrans>(app_path.data(), pvd_id.data(), IAliasPVD::convert(pvd_type).data());
        default:
            LOGERR("Not Supported Provider-Type.(%u)", pvd_type);
            throw std::out_of_range("Not Supported Provider-Type.");
        }
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return std::shared_ptr<IAliasPVD>();
}


/***************************************************
 * Private Function Definition.
 ***************************************************/
bool CConfigAliases::init(const std::string config_path) {
    try{
        // parse json file.
        Json_DataType json_manager = std::make_shared<json_mng::CMjson>();
        assert( json_manager->parse(config_path) == true);
        LOGD("aliases-descriptor Json-file parsing is successful.");

        // build alias-struct.
        auto resources = json_manager->get_member< std::shared_ptr<json_mng::CMjson> >(CONFIG_ALIAS_LIST);
        for ( auto itr = resources->begin(); itr != resources->end(); itr++ ) {

            // build alias for resource/app
            std::string rsc_name = json_mng::CMjson::get_first(itr);
            auto obj_value = json_mng::CMjson::get_second< std::shared_ptr<json_mng::CMjson> >(itr);
            append_rsc_alias( rsc_name, obj_value );
        }
        return true;
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return false;
}

bool CConfigAliases::append_rsc_alias( std::string& rsc_name, std::shared_ptr<json_mng::CMjson> obj_ ) {
    try {
        std::shared_ptr<CAliasRSC> context;
        std::shared_ptr<CAliasAPP> self_app;
        std::shared_ptr<Sproperties> property;
        if( rsc_name.empty() == true || obj_.get() == NULL ) {
            LOGERR("rsc_name=(0x%X), obj_ point(0x%X)", rsc_name.empty(), obj_.get());
            throw std::invalid_argument("rsc name or obj_ is inavlid value.");
        }

        // make CAliasRSC
        auto app = make_app_alias( rsc_name, obj_, self_app);
        context = std::dynamic_pointer_cast<CAliasRSC>( app );
        if( context.get() == NULL ) {
            throw std::runtime_error("CAliasRSC memory-allocation is failed.");
        }

        // register resource
        assert( self_app.get() != NULL );
        context->set_self( self_app );
        _mm_rscs_[rsc_name] = context;
        return true;
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return false;
}

std::shared_ptr<CAliasAPP> CConfigAliases::make_app_alias( std::string& name_, 
                                                            std::shared_ptr<json_mng::CMjson>& obj_, 
                                                            std::shared_ptr<CAliasAPP>& self_,
                                                            std::shared_ptr<CAliasAPP> parent_ ) {
    std::shared_ptr<CAliasAPP> app;
    try {
        std::shared_ptr<Sproperties> property;

        if( name_.empty() == true || obj_.get() == NULL ) {
            LOGERR("name_.empty=%u, obj_=0x%X", name_.empty(), obj_.get());
            throw std::invalid_argument("Invalid argument is inputed.");
        }

        property = get_property( obj_ );
        if( parent_.get() != NULL ) {
            LOGD("App name(%s)", name_.data());
            app = std::make_shared<CAliasAPP>( name_.data(), property->type.data(), parent_ );
        }
        else {
            LOGD("Resource name(%s)", name_.data());
            app = std::make_shared<CAliasRSC>( name_.data(), property->type.data() );
        }

        if( app.get() == NULL ) {
            throw std::runtime_error("CAliasAPP memory-allocation is failed.");
        }

        switch( app->type() ) {
        case enum_c::AppType::E_APPT_SINGLE:
            {
                app->set_where( property->where );
                auto sub_obj = obj_->get_member< std::shared_ptr<json_mng::CMjson> >(CONFIG_ALIAS_PVD_LIST);
                if( sub_obj.get() == NULL ) {
                    throw std::runtime_error("sub_obj(PVD_LIST) is NULL. memory allocation is failed.");
                }

                for( auto itr=sub_obj->begin(); itr != sub_obj->end(); itr++ ) {
                    if( append_pvd_alias( itr, app ) == false ) {
                        throw std::runtime_error("append_pvd_alias is failed.");
                    }
                }
                self_ = app;
            }
            break;
        case enum_c::AppType::E_APPT_MULTIPLE:
            {
                std::string name;
                std::shared_ptr<CAliasAPP> sub_app;
                std::shared_ptr<json_mng::CMjson> obj_value;

                auto sub_obj = obj_->get_member< std::shared_ptr<json_mng::CMjson> >(CONFIG_ALIAS_APP_LIST);
                if( sub_obj.get() == NULL ) {
                    throw std::runtime_error("sub_obj(APP_LIST) is NULL. memory allocation is failed.");
                }

                for( auto itr=sub_obj->begin(); itr != sub_obj->end(); itr++ ) {
                    name = json_mng::CMjson::get_first(itr);
                    obj_value = json_mng::CMjson::get_second< std::shared_ptr<json_mng::CMjson> >(itr);
                    if( property->name == name ) {
                        sub_app = make_app_alias( name, obj_value, self_, app );
                    }
                    else {
                        std::shared_ptr<CAliasAPP> dumy;
                        sub_app = make_app_alias( name, obj_value, dumy, app);
                    }
                    // register app to list.
                    app->set( name, sub_app );
                }
            }
            break;
        default:
            LOGERR("Not supported AppType(%u)", app->type());
            throw std::invalid_argument("Invalid AppType.");
        }
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        app.reset();
        throw e;
    }

    return app;
}

std::shared_ptr<Sproperties> CConfigAliases::get_property( std::shared_ptr<json_mng::CMjson>& obj_ ) {
    std::shared_ptr<Sproperties> property;
    try {
        auto sub_obj = obj_->get_member< std::shared_ptr<json_mng::CMjson> >(CONFIG_ALIAS_PROPERTY);
        if( sub_obj.get() == NULL ) {
            throw std::runtime_error("sub_obj is NULL. memory allocation is failed.");
        }
        property = std::make_shared<Sproperties>();
        if( property.get() == NULL ) {
            throw std::runtime_error("property is NULL. memory allocation is failed.");
        }
        property->type = sub_obj->get_member(CONFIG_ALIAS_PROP_TYPE);
        property->name = sub_obj->get_member(CONFIG_ALIAS_PROP_NAME);
        property->where = sub_obj->get_member(CONFIG_ALIAS_PROP_WHERE);

        // check invalid-verification.
        if( property->type == SINGLE && property->where.empty() == true ) {
            throw std::invalid_argument("where is empty. It's invalid.");
        }

        if( property->type == MULTIPLE && property->name.empty() == true ) {
            throw std::invalid_argument("name is empty. It's invalid.");
        }
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return property;
}

bool CConfigAliases::append_pvd_alias( json_mng::MemberIterator& itr_, std::shared_ptr<CAliasAPP>& app_ ) {
    try {
        std::string pvd_type;
        std::shared_ptr<IAliasPVD> context;
        std::string pvd_alias = json_mng::CMjson::get_first(itr_);
        auto obj_value = json_mng::CMjson::get_second< std::shared_ptr<json_mng::CMjson> >(itr_);
        
        if( app_.get() == NULL || pvd_alias.empty() == true || obj_value.get() == NULL ) {
            LOGERR("app_=0x%X, pvd_alias.empty=%u, obj_=0x%X", app_.get(), pvd_alias.empty(), obj_value.get());
            throw std::runtime_error("There are invalid argument.");
        }

        LOGD("= provider-alias Name=%s", pvd_alias.data());
        context = make_pvd_alias(pvd_alias, obj_value, pvd_type, app_);
        
        // register provider to list.
        app_->set( pvd_alias, context );
        append_pvd_context( app_->path(), pvd_type, context );
        return true;
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return false;
}

void CConfigAliases::append_pvd_context( std::string&& app_path, std::string& pvd_type, std::shared_ptr<IAliasPVD>& context ) {
    try {
        auto itr = _mm_pvds_map_.find( app_path );
        if( itr == _mm_pvds_map_.end() ) {
            _mm_pvds_map_[ app_path ].clear();

            for( int i=0; _ma_pvd_types_[i] != NULL; i++ ) {
                LOGD("APP(%s) PVD-List initialize.(name=%s)", app_path.data(), _ma_pvd_types_[i]);
                _mm_pvds_map_[ app_path ][_ma_pvd_types_[i]].clear();
            }
        }

        _mm_pvds_map_[ app_path ][pvd_type].push_back(context);
        _mm_pvds_[pvd_type].push_back(context);
        LOGI("PVD(%s) is saved to PVD-List.", context->path().data());
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

std::shared_ptr<IAliasPVD> CConfigAliases::make_pvd_alias(std::string alias, 
                                                         std::shared_ptr<json_mng::CMjson> obj_, 
                                                         std::string &pvd_type,
                                                         std::shared_ptr<CAliasAPP>& parent_) {
    std::shared_ptr<IAliasPVD> pvd;
    assert(alias.empty() == false);
    assert(obj_.get() != NULL);

    try {
        pvd_type = obj_->get_member(CONFIG_ALIAS_PVD_TYPE);
        auto obj_value = obj_->get_member< std::shared_ptr<json_mng::CMjson> >(CONFIG_ALIAS_ADDR);
        if( obj_value.get() == NULL ) {
            throw std::logic_error("CONFIG_ALIAS_ADDR is not exist. Please check desp_alias.json file.");
        }

        LOGD("= pvd_type=%s", pvd_type.c_str());
        if ( pvd_type == IAliasPVD::UDP || 
             pvd_type == IAliasPVD::UDP_UDS || 
             pvd_type == IAliasPVD::TCP ||
             pvd_type == IAliasPVD::TCP_UDS ) {
            pvd = make_pvd_trans(alias, pvd_type, obj_value, parent_);
        }
        else if ( pvd_type == IAliasPVD::VSOMEIP ) {
            std::shared_ptr<CAliasService> svc_pvd = make_pvd_service(alias, pvd_type, obj_value, parent_);

            obj_value = obj_->get_member< std::shared_ptr<json_mng::CMjson> >(CONFIG_ALIAS_FUNCTION);
            set_pvd_service_with_function( svc_pvd, obj_value );
            pvd = svc_pvd;
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

    return pvd;
}

std::shared_ptr<CAliasTrans> CConfigAliases::make_pvd_trans(std::string alias, 
                                                           std::string pvd_type, 
                                                           std::shared_ptr<json_mng::CMjson> &obj_addr,
                                                           std::shared_ptr<CAliasAPP>& parent_) {
    assert(alias.empty() == false);
    assert(pvd_type.empty() == false);
    assert(obj_addr.get() != NULL);
    std::shared_ptr<CAliasTrans> res = std::make_shared<CAliasTrans>(alias.c_str(), pvd_type.c_str(), parent_);

    try {
        // extract 'address' part.
        for ( auto itr = obj_addr->begin(); itr != obj_addr->end(); itr++ ) {
            std::string key = json_mng::CMjson::get_first(itr);
            assert( key.empty() == false );
            LOGD("= Key=%s", key.c_str());

            if ( key == CONFIG_ALIAS_IP ) {
                res->set_ip( json_mng::CMjson::get_second(itr) );
                LOGD("= Value=%s", res->get_ip().c_str());
            }
            else if ( key == CONFIG_ALIAS_PORT ) {
                res->set_port( json_mng::CMjson::get_second<uint32_t>(itr) );
                LOGD("= Value=%d", res->get_port());
            }
            else if ( key == CONFIG_ALIAS_MASK ) {
                res->set_mask( json_mng::CMjson::get_second<uint32_t>(itr) );
                LOGD("= Value=%d", res->get_mask());
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

std::shared_ptr<CAliasService> CConfigAliases::make_pvd_service(std::string alias, 
                                                               std::string pvd_type, 
                                                               std::shared_ptr<json_mng::CMjson> &obj_addr,
                                                               std::shared_ptr<CAliasAPP>& parent_) {
    assert(alias.empty() == false);
    assert(pvd_type.empty() == false);
    assert(obj_addr.get() != NULL);
    std::shared_ptr<CAliasService> res = std::make_shared<CAliasService>(alias.c_str(), pvd_type.c_str(), parent_);

    try {
        // extract 'address' part.
        for ( auto itr = obj_addr->begin(); itr != obj_addr->end(); itr++ ) {
            std::string key = json_mng::CMjson::get_first(itr);
            assert( key.empty() == false );
            LOGD("= Key=%s", key.c_str());

            if ( key == CONFIG_ALIAS_SVC_ID ) {
                res->set_svc_id( json_mng::CMjson::get_second_hex<uint32_t>(itr) );
                LOGD("= Value=%u", res->get_svc_id());
            }
            else if ( key == CONFIG_ALIAS_INST_ID ) {
                res->set_inst_id( json_mng::CMjson::get_second_hex<uint32_t>(itr) );
                LOGD("= Value=%u", res->get_inst_id());
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

void CConfigAliases::set_pvd_service_with_function( std::shared_ptr<CAliasService>& pvd_, 
                                                    std::shared_ptr<json_mng::CMjson> &obj_ ) {
    try {
        if( pvd_.get() == NULL ) {
            throw std::invalid_argument("CAliasService is NULL.");
        }
        
        if( obj_.get() == NULL ) {
            throw std::invalid_argument("CONFIG_ALIAS_FUNCTION is not exist. Please check desp_alias.json file.");
        }

        // Define lamda to parsing json.
        auto lamda_parsing_functions = [&]( const char* func_key_, std::function<void(std::string&, std::shared_ptr<json_mng::CMjson>&)> cb_sub_routine_ ) -> void {
            if( func_key_ == NULL || cb_sub_routine_ == nullptr ) {
                LOGERR("func_key_=0x%X, cb_sub_routine_=0x%X", func_key_, cb_sub_routine_);
                throw std::invalid_argument("func_key or cb_sub_routine_ is NULL.");
            }

            auto obj_value = obj_->get_member< std::shared_ptr<json_mng::CMjson> >(func_key_);
            if( obj_value.get() == NULL ) {
                LOGERR("There are not exist Key(%s) in desp_alias.json file.", func_key_);
                throw std::logic_error("KEY is not exist. Please check desp_alias.json file.");
            }

            for( auto itr=obj_value->begin(); itr!=obj_value->end(); itr++ ) {
                std::string func_name = json_mng::CMjson::get_first(itr);
                auto sub_obj = json_mng::CMjson::get_second< std::shared_ptr<json_mng::CMjson> >(itr);

                if( func_name.empty() == true || sub_obj.get() == nullptr ) {
                    throw std::runtime_error("There are invalid values.");
                }

                cb_sub_routine_( func_name, sub_obj );
            }
        };

        // run parsing routine.
        lamda_parsing_functions( CONFIG_ALIAS_REQ_RESP, [&](std::string& name_, std::shared_ptr<json_mng::CMjson>& sub_obj_)->void {
            uint32_t id = sub_obj_->get_member_hex<uint32_t>(CONFIG_ALIAS_FUNC_ID);
            pvd_->push_reqresp_id( name_, id );
        });

        lamda_parsing_functions( CONFIG_ALIAS_REQ_NO_RESP, [&](std::string& name_, std::shared_ptr<json_mng::CMjson>& sub_obj_)->void {
            uint32_t id = sub_obj_->get_member_hex<uint32_t>(CONFIG_ALIAS_FUNC_ID);
            pvd_->push_reqNOresp_id( name_, id );
        });

        lamda_parsing_functions( CONFIG_ALIAS_PUB_SUB, [&](std::string& name_, std::shared_ptr<json_mng::CMjson>& sub_obj_)->void {
            uint32_t id = sub_obj_->get_member_hex<uint32_t>(CONFIG_ALIAS_FUNC_ID);
            uint32_t grp_id = sub_obj_->get_member_hex<uint32_t>(CONFIG_ALIAS_FUNC_GID);
            pvd_->push_pubsub_id( name_, id, grp_id );
        });
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}


/*************************
 * CAliasAPP Class Definition
 */
CAliasAPP::CAliasAPP(const char* alias_, const char* type_, std::shared_ptr<IAlias> parent_)
: IAlias( alias_, enum_c::AliasType::E_ALIAST_APP, parent_ ) {
    assert( type_ != NULL );
    
    try{
        _m_type_ = convert(type_);
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

CAliasAPP::~CAliasAPP(void) {
    _m_type_ = enum_c::AppType::E_APPT_NOT_DEFINE;
}

enum_c::AppType CAliasAPP::type( void ) {
    return _m_type_;
}

std::string& CAliasAPP::get_where( void ) {
    return _m_where_;
}

void CAliasAPP::set_where( std::string& where_ ) {
    _m_where_ = where_;
}

std::shared_ptr<IAlias> CAliasAPP::get( std::string& alias_ ) {
    std::shared_ptr<IAlias> res;
    try {
        if( alias_.empty() == true ) {
            throw std::invalid_argument("alias-name is empty.");
        }

        auto itr = _mm_alias_.find( alias_ );
        if( itr != _mm_alias_.end() ) {
            res = itr->second;
        }
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return res;
}

void CAliasAPP::set( std::string& alias_, std::shared_ptr<IAlias> context_ ) {
    try {
        if( get(alias_).get() != NULL ) {
            LOGERR("Already, IAlias exist about key(%s).", alias_.data());
            return ;
        }

        _mm_alias_[alias_] = context_;
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

enum_c::AppType CAliasAPP::convert(std::string type_str) {
    assert(type_str.empty() == false);

    if(type_str == CConfigAliases::SINGLE) {
        return enum_c::AppType::E_APPT_SINGLE;
    }
    else if(type_str == CConfigAliases::MULTIPLE) {
        return enum_c::AppType::E_APPT_MULTIPLE;
    }
    else {
        throw CException(E_ERROR::E_NOT_SUPPORTED_APP_TYPE);
    }
}


/***
 * CAliasTrans class definition.
 */
CAliasTrans::CAliasTrans(const char* alias, const char* pvd_type, std::shared_ptr<IAlias> parent_)
: IAliasPVD(alias, pvd_type, parent_) {
    clear();
}

CAliasTrans::CAliasTrans(const char* app_path, const char* pvd_id, const char* pvd_type)
: IAliasPVD(app_path, pvd_id, pvd_type) {
    clear();
}

CAliasTrans::~CAliasTrans(void) {
    clear();
}

// getter
std::string CAliasTrans::get_ip( void ) {
    return _m_ip_;
}

uint32_t CAliasTrans::get_mask( void ) {
    return _m_mask_;
}

uint32_t CAliasTrans::get_port( void ) {
    return _m_port_num_;
}

std::string& CAliasTrans::get_ip_ref( void ) {
    return _m_ip_;
}

uint32_t& CAliasTrans::get_port_ref( void ) {
    return _m_port_num_;
}

// setter
void CAliasTrans::set_ip( std::string ip_ ) {
    _m_ip_ = ip_;
}

void CAliasTrans::set_mask( uint32_t mask_ ) {
    _m_mask_ = mask_;
}

void CAliasTrans::set_port( uint32_t port_ ) {
    _m_port_num_ = port_;
}

void CAliasTrans::clear( void ) {
    _m_ip_.clear();
    _m_port_num_ = 0;
    _m_mask_ = 0;
}


/***
 * CAliasService class definition.
 */
CAliasService::CAliasService(const char* alias, const char* pvd_type, std::shared_ptr<IAlias> parent_)
: IAliasPVD(alias, pvd_type, parent_) {
    clear();
}

CAliasService::CAliasService(const char* app_path, const char* pvd_id, const char* pvd_type)
: IAliasPVD(app_path, pvd_id, pvd_type) {
    clear();
}

CAliasService::~CAliasService(void) {
    clear();
}

// getter
uint32_t CAliasService::get_svc_id( void ) {
    return _m_svc_id_;
}

uint32_t CAliasService::get_inst_id( void ) {
    return _m_inst_id_;
}

// setter
void CAliasService::set_svc_id( uint32_t svc_id_ ) {
    _m_svc_id_ = svc_id_;
}

void CAliasService::set_inst_id( uint32_t inst_id_ ) {
    _m_inst_id_ = inst_id_;
}

void CAliasService::push_reqresp_id( std::string& name_, uint32_t id_ ) {
    try {
        Sreq_resp method = { id_ };
        if( name_.empty() == true || id_ == NULL ) {
            throw std::invalid_argument("Invalid argument.");
        }

        auto itr = _mm_req_resp_.find(name_);
        if( itr != _mm_req_resp_.end() ) {
            LOGW("Already exist Sreq_resp(method) as name(%s)", name_.data());
            throw std::invalid_argument("Already exist Sreq_resp(method)");
        }

        _mm_req_resp_[name_] = method;
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

void CAliasService::push_reqNOresp_id( std::string& name_, uint32_t id_ ) {
    try {
        Sreq_resp method = { id_ };
        if( name_.empty() == true || id_ == NULL ) {
            throw std::invalid_argument("Invalid argument.");
        }

        auto itr = _mm_req_noresp_.find(name_);
        if( itr != _mm_req_noresp_.end() ) {
            LOGW("Already exist Sreq_resp(method) as name(%s)", name_.data());
            throw std::invalid_argument("Already exist Sreq_resp(method)");
        }

        _mm_req_noresp_[name_] = method;
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

void CAliasService::push_pubsub_id( std::string& name_, uint32_t id_, uint32_t grp_id_ ) {
    try {
        Spub_sub event = { id_ , grp_id_ };
        if( name_.empty() == true || id_ == NULL || grp_id_ == NULL ) {
            throw std::invalid_argument("Invalid argument.");
        }

        auto itr = _mm_pub_sub_.find(name_);
        if( itr != _mm_pub_sub_.end() ) {
            LOGW("Already exist Spub_sub(event) as name(%s)", name_.data());
            throw std::invalid_argument("Already exist Spub_sub(event)");
        }

        _mm_pub_sub_[name_] = event;
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

void CAliasService::clear( void ) {
    _m_svc_id_ = 0;
    _m_inst_id_ = 0;
    
    _mm_req_resp_.clear();
    _mm_req_noresp_.clear();
    _mm_pub_sub_.clear();
}



}   // namespace cf_alias
