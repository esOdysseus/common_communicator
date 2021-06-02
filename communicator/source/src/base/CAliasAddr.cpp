/***
 * CAliasAddr.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <cassert>
#include <netinet/in.h>
#include <sys/un.h>
#include <memory.h>
#include <mutex>

#include <logger.h>
#include <CAliasAddr.h>
#include <CAliasCompare.h>

template class CAliasAddr<struct sockaddr_in, CALIAS_CMPFUNC<struct sockaddr_in>>;
template class CAliasAddr<struct sockaddr_un, CALIAS_CMPFUNC<struct sockaddr_un>>;
template class CAliasAddr<int>;


/*********************************
 * Definition of Public Function.
 */
template <typename ADDR_TYPE, typename _Compare>
CAliasAddr<ADDR_TYPE,_Compare>::CAliasAddr(void) {
    clear();
}

template <typename ADDR_TYPE, typename _Compare>
CAliasAddr<ADDR_TYPE,_Compare>::~CAliasAddr(void) {
    clear();
}

template <typename ADDR_TYPE, typename _Compare>
bool CAliasAddr<ADDR_TYPE,_Compare>::is_there(std::string pvd_full_path) {
    return ( _mm_addr_.find(pvd_full_path) != _mm_addr_.end() ? true : false );
}

template <typename ADDR_TYPE, typename _Compare>
bool CAliasAddr<ADDR_TYPE,_Compare>::is_there(const ADDR_TYPE &addr) {
    return ( _mm_alias_.find(addr) != _mm_alias_.end() ? true : false );
}

template <typename ADDR_TYPE, typename _Compare>
std::shared_ptr<ADDR_TYPE> CAliasAddr<ADDR_TYPE,_Compare>::get(std::string pvd_full_path) {
    std::shared_ptr<ADDR_TYPE> res;
    AddrIterator itor_addr;

    try {
        std::shared_lock<std::shared_mutex> guard(mtx_sync);

        // Find Address correspond with pvd_full_path.
        if ( (itor_addr = _mm_addr_.find(pvd_full_path)) != _mm_addr_.end() ) {
            res = (itor_addr->second).get_address<ADDR_TYPE>();
        }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }

    return res;
}

template <typename ADDR_TYPE, typename _Compare>
const ADDR_TYPE * CAliasAddr<ADDR_TYPE,_Compare>::get_read_only(std::string pvd_full_path) {
    return (const ADDR_TYPE *)(get(pvd_full_path).get());
}

template <typename ADDR_TYPE, typename _Compare>
std::shared_ptr<cf_alias::IAliasPVD> CAliasAddr<ADDR_TYPE,_Compare>::get(const ADDR_TYPE &addr) {
    return get(std::forward<const ADDR_TYPE>(addr));
}

template <typename ADDR_TYPE, typename _Compare>
std::shared_ptr<cf_alias::IAliasPVD> CAliasAddr<ADDR_TYPE,_Compare>::get(const ADDR_TYPE &&addr) {
    AliasIterator itor_alias;

    try {
        std::shared_lock<std::shared_mutex> guard(mtx_sync);
        // Find Address correspond with Alias-naming.
        if ( (itor_alias = _mm_alias_.find(addr)) != _mm_alias_.end() ) {
            return itor_alias->second;
        }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }

    return std::shared_ptr<cf_alias::IAliasPVD>();
}

template <typename ADDR_TYPE, typename _Compare>
bool CAliasAddr<ADDR_TYPE,_Compare>::insert(
                        std::shared_ptr<cf_alias::IAliasPVD> pvd_alias, 
                        const ADDR_TYPE &address, 
                        bool &is_new, bool connect_flag) {
    bool result = false;
    std::shared_ptr<ADDR_TYPE> addr;

    try {
        addr = std::make_shared<ADDR_TYPE>();
        assert(addr.get() != NULL);
        memcpy(addr.get(), &address, sizeof(ADDR_TYPE));

        result = insert( pvd_alias, addr, is_new, connect_flag );
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }
    return result;
}

template <typename ADDR_TYPE, typename _Compare>
bool CAliasAddr<ADDR_TYPE,_Compare>::insert(
                        std::shared_ptr<cf_alias::IAliasPVD> pvd_alias, 
                        std::shared_ptr<ADDR_TYPE> &address, 
                        bool &is_new, bool connect_flag) {
    bool result = false;
    // ADDR_TYPE addr;

    try {
        if( pvd_alias.get() == NULL ) {
            return result;
        }
        std::string pvd_full_path = pvd_alias->path();

        is_new = false;
        // memcpy(&addr, address.get(), sizeof(ADDR_TYPE));
        if ( _mm_addr_.find(pvd_full_path) == _mm_addr_.end() || 
             _mm_addr_.find(pvd_full_path)->second.get_connect_flag() == false ) {

            std::lock_guard<std::shared_mutex> guard(mtx_sync);

            auto itor = _mm_addr_.find(pvd_full_path);
            if ( itor == _mm_addr_.end() ) {
                assert( _mm_alias_.find( *address ) == _mm_alias_.end() );
                CSource* src = &(_mm_addr_[pvd_full_path]);
                assert(src != NULL);

                src->init(address, pvd_alias->path_parent().data(), pvd_alias->name().data(), pvd_alias->type(), connect_flag);
                _mm_alias_.insert( std::make_pair(*address, pvd_alias) );
                is_new = true;
            }
            else if (itor->second.get_connect_flag() == false) {
                itor->second.set_connect_flag(connect_flag);
                is_new = connect_flag;
            }
        }
        result = true;
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }
    return result;
}

template <typename ADDR_TYPE, typename _Compare>
void CAliasAddr<ADDR_TYPE,_Compare>::remove(std::string pvd_full_path) {
    std::shared_ptr<ADDR_TYPE> addr = get(pvd_full_path);
    
    try {
        if ( addr.get() != NULL ) {
            remove_impl( pvd_full_path, *(addr.get()) );
        }
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

template <typename ADDR_TYPE, typename _Compare>
void CAliasAddr<ADDR_TYPE,_Compare>::remove(ADDR_TYPE &addr) {
    std::shared_ptr<cf_alias::IAliasPVD> pvd_alias = get(addr);

    try {
        if ( pvd_alias.get() != NULL ) {
            remove_impl(pvd_alias->name(), addr);
        }
    }
    catch ( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

template <typename ADDR_TYPE, typename _Compare>
void CAliasAddr<ADDR_TYPE,_Compare>::remove_impl(std::string pvd_full_path, ADDR_TYPE addr) {
    AddrIterator itor_addr;
    AliasIterator itor_alias;
    assert( pvd_full_path.empty() == false );

    try {
        std::lock_guard<std::shared_mutex> guard(mtx_sync);

        itor_addr = _mm_addr_.find(pvd_full_path);
        if (itor_addr != _mm_addr_.end()) {
            _mm_addr_.erase(itor_addr);
        }

        itor_alias = _mm_alias_.find( addr );
        if (itor_alias != _mm_alias_.end()) {
            _mm_alias_.erase(itor_alias);
        }
    }
    catch ( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

template <typename ADDR_TYPE, typename _Compare>
void CAliasAddr<ADDR_TYPE,_Compare>::clear(void) {
    _mm_addr_.clear();
    _mm_alias_.clear();
}

template <typename ADDR_TYPE, typename _Compare>
void CAliasAddr<ADDR_TYPE,_Compare>::reset_connect_flag(std::string pvd_full_path, bool &past_flag) {
    try {
        std::lock_guard<std::shared_mutex> guard(mtx_sync);

        auto itor = _mm_addr_.find(pvd_full_path);
        if( itor != _mm_addr_.end() ) {
            past_flag = itor->second.get_connect_flag();
            itor->second.set_connect_flag(false);
        }
    }
    catch ( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}