#include <cassert>

#include <logger.h>
#include <CAliasAddr.h>

template std::shared_ptr<struct sockaddr_in> CAliasAddr::get(std::string alias);
template std::shared_ptr<int> CAliasAddr::get(std::string alias);
template const struct sockaddr_in* CAliasAddr::get_read_only(std::string alias);
template const int* CAliasAddr::get_read_only(std::string alias);
template bool CAliasAddr::insert(std::string alias, std::shared_ptr<struct sockaddr_in> &address, enum_c::ProviderType pvd_type, bool &is_new);
template bool CAliasAddr::insert(std::string alias, std::shared_ptr<int> &address, enum_c::ProviderType pvd_type, bool &is_new);


/*********************************
 * Definition of Public Function.
 */
CAliasAddr::CAliasAddr(void) {
    clear();
}

CAliasAddr::~CAliasAddr(void) {
    clear();
}

bool CAliasAddr::is_there(std::string alias) {
    return ( map_addr.find(alias) != map_addr.end() ? true : false );
}

template <typename ADDR_TYPE> 
std::shared_ptr<ADDR_TYPE> CAliasAddr::get(std::string alias) {
    std::shared_ptr<ADDR_TYPE> res;

    try {
        // Find Address correspond with Alias-naming.
        if ( map_addr.find(alias) != map_addr.end() ) {
            res = (map_addr[alias]).get_address<ADDR_TYPE>();
        }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }

    return res;
}

template <typename ADDR_TYPE> 
const ADDR_TYPE * CAliasAddr::get_read_only(std::string alias) {
    return (const ADDR_TYPE *)(get<ADDR_TYPE>(alias).get());
}

template <typename ADDR_TYPE> 
bool CAliasAddr::insert(std::string alias, 
                        std::shared_ptr<ADDR_TYPE> &address, 
                        enum_c::ProviderType pvd_type, 
                        bool &is_new) {
    bool result = false;

    try {
        if( alias.empty() == true ) {
            return result;
        }

        is_new = false;
        if ( map_addr.find(alias) == map_addr.end() ) {
            CSource* src = &(map_addr[alias]);
            assert(src != NULL);

            src->init(address, alias.c_str(), pvd_type);
            // map_addr[alias] = std::make_shared<struct sockaddr_in>();
            // struct sockaddr_in* p_addr = map_addr[alias].get();
            // memcpy(p_addr, &address, sizeof(address));
            is_new = true;
        }
        result = true;
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }
    return result;
}

void CAliasAddr::remove(std::string alias) {
    AddressMapType::iterator itor = map_addr.find(alias);
    if (itor != map_addr.end()) {
        map_addr.erase(itor);
    }
}

void CAliasAddr::clear(void) {
    map_addr.clear();
}