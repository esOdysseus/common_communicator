/***
 * IAliasPVD.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <cassert>

#include <logger.h>
#include <IAliasPVD.h>

namespace cf_alias {

constexpr const char* IAliasPVD::UDP;
constexpr const char* IAliasPVD::UDP_UDS;
constexpr const char* IAliasPVD::TCP;
constexpr const char* IAliasPVD::TCP_UDS;
constexpr const char* IAliasPVD::VSOMEIP;


/***
 * IAlias class definition.
 */
IAlias::IAlias(const char* alias_, enum_c::AliasType type_, std::shared_ptr<IAlias> parent_) {
    assert( alias_ != NULL );
    _m_name_ = alias_;
    _m_type_ = type_;
    _m_path_.clear();
    if( parent_.get() != NULL ) {
        set_path_parent( *parent_.get() );
    }
}

IAlias::IAlias(const char* parent_app_path, const char* alias_, enum_c::AliasType type_) {
    assert( parent_app_path != NULL );
    assert( alias_ != NULL );
    _m_name_ = alias_;
    _m_type_ = type_;
    _m_path_.clear();
    set_path_parent( parent_app_path );
}

IAlias::~IAlias(void) {
    _m_name_.clear();
    _m_path_.clear();
    _m_type_ = enum_c::AliasType::E_ALIAST_NOT_DEFINE;
}

// getter
std::string IAlias::name( void ) {
    return _m_name_;
}

std::string IAlias::path( void ) {
    return IAlias::make_full_path(_m_path_, _m_name_);
}

std::string IAlias::path_parent( void ) {
    return _m_path_;
}

enum_c::AliasType IAlias::alias_type( void ) {
    return _m_type_;
}

std::string IAlias::make_full_path(std::string &app_path, std::string &pvd_path) {
    if( app_path.empty() != true ) {
        return app_path + '/' + pvd_path;
    }

    return pvd_path;
}

std::string IAlias::make_full_path(std::string &&app_path, std::string &&pvd_path) {
    if( app_path.empty() != true ) {
        return app_path + '/' + pvd_path;
    }

    return pvd_path;
}

// setter
void IAlias::set_path_parent( IAlias& parent_ ) {
    _m_path_ = parent_.path();
}

void IAlias::set_path_parent( std::string app_path ) {
    _m_path_ = std::move(app_path);
}


/*************************
 * IAliasPVD Class Definition
 */
IAliasPVD::IAliasPVD(const char* pvd_id, const char* pvd_type_, std::shared_ptr<IAlias> parent_)
: IAlias( pvd_id, enum_c::AliasType::E_ALIAST_PROVIDER, parent_ ) {
    assert( pvd_type_ != NULL );
    
    try{
        _m_type_ = convert(pvd_type_);
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

IAliasPVD::IAliasPVD(const char* app_path, const char* pvd_id, const char* pvd_type_)
: IAlias( app_path, pvd_id, enum_c::AliasType::E_ALIAST_PROVIDER ) {
    assert( pvd_type_ != NULL );
    
    try{
        _m_type_ = convert(pvd_type_);
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

IAliasPVD::~IAliasPVD(void) {
    _m_type_ = enum_c::ProviderType::E_PVDT_NOT_DEFINE;
}

enum_c::ProviderType IAliasPVD::type( void ) {
    return _m_type_;
}

std::string IAliasPVD::convert(enum_c::ProviderType pvd_type) {
    std::string res;

    switch( pvd_type ) {
    case enum_c::ProviderType::E_PVDT_TRANS_UDP:
        res = std::string(IAliasPVD::UDP);
        break;
    case enum_c::ProviderType::E_PVDT_TRANS_UDS_UDP:
        res = std::string(IAliasPVD::UDP_UDS);
        break;
    case enum_c::ProviderType::E_PVDT_TRANS_TCP:
        res = std::string(IAliasPVD::TCP);
        break;
    case enum_c::ProviderType::E_PVDT_TRANS_UDS_TCP:
        res = std::string(IAliasPVD::TCP_UDS);
        break;
    case enum_c::ProviderType::E_PVDT_SERVICE_VSOMEIP:
        res = std::string(IAliasPVD::VSOMEIP);
        break;
    default:
        {
            std::string err_str = "Not Support Provider-Type.(" + std::to_string(pvd_type) + ")";
            throw std::out_of_range(err_str);
        }
    }

    return res;
}

enum_c::ProviderType IAliasPVD::convert(std::string pvd_type_str) {
    assert(pvd_type_str.empty() == false);

    if(pvd_type_str == IAliasPVD::UDP) {
        return enum_c::ProviderType::E_PVDT_TRANS_UDP;
    }
    else if(pvd_type_str == IAliasPVD::UDP_UDS) {
        return enum_c::ProviderType::E_PVDT_TRANS_UDS_UDP;
    }
    else if(pvd_type_str == IAliasPVD::TCP) {
        return enum_c::ProviderType::E_PVDT_TRANS_TCP;
    }
    else if(pvd_type_str == IAliasPVD::TCP_UDS) {
        return enum_c::ProviderType::E_PVDT_TRANS_UDS_TCP;
    }
    else if(pvd_type_str == IAliasPVD::VSOMEIP) {
        return enum_c::ProviderType::E_PVDT_SERVICE_VSOMEIP;
    }
    else {
        std::string err_str = "Not Support Provider-Type.(" + pvd_type_str + ")";
        throw std::out_of_range(err_str);
    }
}



}   // namespace cf_alias
