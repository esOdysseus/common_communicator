#include <cassert>
#include <netinet/in.h>
#include <memory.h>
#include <mutex>

#include <logger.h>
#include <CAliasAddr.h>
#include <CAliasCompare.h>

template class CAliasAddr<struct sockaddr_in, CALIAS_CMPFUNC_for_sockaddr_in>;
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
bool CAliasAddr<ADDR_TYPE,_Compare>::is_there(std::string alias) {
    return ( map_addr.find(alias) != map_addr.end() ? true : false );
}

template <typename ADDR_TYPE, typename _Compare>
bool CAliasAddr<ADDR_TYPE,_Compare>::is_there(const ADDR_TYPE &addr) {
    return ( map_alias.find(addr) != map_alias.end() ? true : false );
}

template <typename ADDR_TYPE, typename _Compare>
std::shared_ptr<ADDR_TYPE> CAliasAddr<ADDR_TYPE,_Compare>::get(std::string alias) {
    std::shared_ptr<ADDR_TYPE> res;
    AddrIterator itor_addr;

    try {
        std::shared_lock<std::shared_mutex> guard(mtx_sync);
        // Find Address correspond with Alias-naming.
        if ( (itor_addr = map_addr.find(alias)) != map_addr.end() ) {
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
const ADDR_TYPE * CAliasAddr<ADDR_TYPE,_Compare>::get_read_only(std::string alias) {
    return (const ADDR_TYPE *)(get(alias).get());
}

template <typename ADDR_TYPE, typename _Compare>
std::string CAliasAddr<ADDR_TYPE,_Compare>::get(const ADDR_TYPE &addr) {
    AliasIterator itor_alias;

    try {
        std::shared_lock<std::shared_mutex> guard(mtx_sync);
        // Find Address correspond with Alias-naming.
        if ( (itor_alias = map_alias.find(addr)) != map_alias.end() ) {
            return itor_alias->second;
        }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }

    return std::string();
}

template <typename ADDR_TYPE, typename _Compare>
bool CAliasAddr<ADDR_TYPE,_Compare>::insert(std::string alias, 
                        std::shared_ptr<ADDR_TYPE> &address, 
                        enum_c::ProviderType pvd_type, 
                        bool &is_new) {
    bool result = false;
    ADDR_TYPE addr;

    try {
        if( alias.empty() == true ) {
            return result;
        }

        is_new = false;
        memcpy(&addr, address.get(), sizeof(ADDR_TYPE));
        if ( map_addr.find(alias) == map_addr.end() ) {
            std::lock_guard<std::shared_mutex> guard(mtx_sync);

            if ( map_addr.find(alias) == map_addr.end() ) {
                assert( map_alias.find( addr ) == map_alias.end() );
                CSource* src = &(map_addr[alias]);
                assert(src != NULL);

                src->init(address, alias.c_str(), pvd_type);
                map_alias.insert( std::make_pair(addr, alias) );
                is_new = true;
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
void CAliasAddr<ADDR_TYPE,_Compare>::remove(std::string alias) {
    std::shared_ptr<ADDR_TYPE> addr = get(alias);
    
    try {
        if ( addr.get() != NULL ) {
            remove_impl( alias, *(addr.get()) );
        }
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

template <typename ADDR_TYPE, typename _Compare>
void CAliasAddr<ADDR_TYPE,_Compare>::remove(ADDR_TYPE &addr) {
    std::string name_alias = get(addr);

    try {
        if ( name_alias.empty() == false ) {
            remove_impl(name_alias, addr);
        }
    }
    catch ( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

template <typename ADDR_TYPE, typename _Compare>
void CAliasAddr<ADDR_TYPE,_Compare>::remove_impl(std::string alias, ADDR_TYPE addr) {
    AddrIterator itor_addr;
    AliasIterator itor_alias;
    assert( alias.empty() == false );

    try {
        std::lock_guard<std::shared_mutex> guard(mtx_sync);

        itor_addr = map_addr.find(alias);
        if (itor_addr != map_addr.end()) {
            map_addr.erase(itor_addr);
        }

        itor_alias = map_alias.find( addr );
        if (itor_alias != map_alias.end()) {
            map_alias.erase(itor_alias);
        }
    }
    catch ( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

template <typename ADDR_TYPE, typename _Compare>
void CAliasAddr<ADDR_TYPE,_Compare>::clear(void) {
    map_addr.clear();
}