/***
 * CSource.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef _CSOURCE_H_
#define _CSOURCE_H_

#include <memory>
#include <string>

#include <Enum_common.h>

class CSource {
public:
    using EADDR_TYPE = enum_c::ProviderType;

public:
    CSource(void);

    ~CSource(void);

    template <typename ADDR_TYPE> 
    void init(std::shared_ptr<ADDR_TYPE> addr, 
              const char* app_path, 
              const char* pvd_path, 
              enum_c::ProviderType provider_type,
              bool connect_flag=false);

    template <typename ADDR_TYPE> 
    std::shared_ptr<ADDR_TYPE> get_address(void);

    EADDR_TYPE get_addr_type(void);

    std::string get_app_path(void);

    std::string get_pvd_path(void);

    bool get_connect_flag(void);

    void set_connect_flag(bool flag);

private:
    std::shared_ptr<void> address;

    std::string _m_app_path_;

    std::string _m_pvd_path_;

    EADDR_TYPE addr_type;

    bool connect_flag;
};

#endif // _CSOURCE_H_