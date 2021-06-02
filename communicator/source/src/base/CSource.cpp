/***
 * CSource.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <netinet/in.h>
#include <sys/un.h>

#include <logger.h>
#include <CSource.h>

template void CSource::init(std::shared_ptr<struct sockaddr_in> addr, const char* app_path, const char* pvd_path, enum_c::ProviderType provider_type, bool connect_flag);
template void CSource::init(std::shared_ptr<struct sockaddr_un> addr, const char* app_path, const char* pvd_path, enum_c::ProviderType provider_type, bool connect_flag);
template void CSource::init(std::shared_ptr<int> addr, const char* app_path, const char* pvd_path, enum_c::ProviderType provider_type, bool connect_flag);
template std::shared_ptr<struct sockaddr_in> CSource::get_address(void);
template std::shared_ptr<struct sockaddr_un> CSource::get_address(void);
template std::shared_ptr<int> CSource::get_address(void);

/***********************************************************
 * Definition member-function of CSource Class.
 * */
CSource::CSource(void) {
    addr_type = EADDR_TYPE::E_PVDT_NOT_DEFINE;
    _m_app_path_.clear();
    _m_pvd_path_.clear();
    address.reset();
    connect_flag = false;
}

CSource::~CSource(void){
    try{
        if(addr_type != EADDR_TYPE::E_PVDT_NOT_DEFINE) {
            address.reset();
        }
        addr_type = EADDR_TYPE::E_PVDT_NOT_DEFINE;
        _m_app_path_.clear();
        _m_pvd_path_.clear();
        connect_flag = false;
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }
}

template <typename ADDR_TYPE> 
void CSource::init( std::shared_ptr<ADDR_TYPE> addr, 
                    const char* app_path, 
                    const char* pvd_path, 
                    enum_c::ProviderType provider_type,
                    bool connect_flag) {
    try {
        std::shared_ptr<void> dumy = std::static_pointer_cast<void>(addr);
        this->address = move(dumy);
        this->addr_type = provider_type;
        this->_m_app_path_ = app_path;
        this->_m_pvd_path_ = pvd_path;
        this->connect_flag = connect_flag;
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }
}

template <typename ADDR_TYPE> 
std::shared_ptr<ADDR_TYPE> CSource::get_address(void) {
    try{
        return std::static_pointer_cast<ADDR_TYPE>(address);
    }catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }
}

CSource::EADDR_TYPE CSource::get_addr_type(void) {
    return addr_type;
}

std::string CSource::get_app_path(void) {
    return _m_app_path_;
}

std::string CSource::get_pvd_path(void) {
    return _m_pvd_path_;
}

bool CSource::get_connect_flag(void) {
    return connect_flag;
}

void CSource::set_connect_flag(bool value) {
    connect_flag = value;
}
