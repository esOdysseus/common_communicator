/***
 * CAliasAddr.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef _C_ALIAS_ADDRESS_H_
#define _C_ALIAS_ADDRESS_H_

#include <map>
#include <memory>
#include <string>
#if __cplusplus > 201402L
    #include <shared_mutex> // for c++17
#else
    #include <shared_mutex_kes.h>   // for c++11 , c++14
#endif

#include <CSource.h>
#include <IAliasPVD.h>
#include <Enum_common.h>

template <typename ADDR_TYPE, typename _Compare = std::less<ADDR_TYPE>> 
class CAliasAddr {
private:
    using AddressMapType = std::map<std::string /*pvd_full_path*/, CSource /*essential-addr*/>;
    using AliasMapType = std::map<ADDR_TYPE /*address*/, std::shared_ptr<cf_alias::IAliasPVD> /*pvd_alias*/, _Compare>;

public:
    using AddrIterator = AddressMapType::iterator;
    using AliasIterator = typename AliasMapType::iterator;

public:
    CAliasAddr(void);

    ~CAliasAddr(void);

    AddrIterator begin(void) { return _mm_addr_.begin(); };

    AddrIterator end(void) { return _mm_addr_.end(); };

    bool is_there(std::string pvd_full_path);

    bool is_there(const ADDR_TYPE &addr);

    std::shared_ptr<ADDR_TYPE> get(std::string pvd_full_path);

    const ADDR_TYPE * get_read_only(std::string pvd_full_path);

    std::shared_ptr<cf_alias::IAliasPVD> get(const ADDR_TYPE &addr);

    std::shared_ptr<cf_alias::IAliasPVD> get(const ADDR_TYPE &&addr);

    bool insert(std::shared_ptr<cf_alias::IAliasPVD> pvd_alias, 
                const ADDR_TYPE &address, 
                bool &is_new, 
                bool connect_flag=false);

    bool insert(std::shared_ptr<cf_alias::IAliasPVD> pvd_alias, 
                std::shared_ptr<ADDR_TYPE> &address, 
                bool &is_new,
                bool connect_flag=false);

    void remove(std::string pvd_full_path);

    void remove(ADDR_TYPE &addr);

    void clear(void);

    void reset_connect_flag(std::string pvd_full_path, bool &past_flag);

private:
    void remove_impl(std::string pvd_full_path, ADDR_TYPE addr);

private:
    AddressMapType _mm_addr_;

    AliasMapType _mm_alias_;

    std::shared_mutex mtx_sync;

};

#endif // _C_ALIAS_ADDRESS_H_