#ifndef _C_ALIAS_ADDRESS_H_
#define _C_ALIAS_ADDRESS_H_

#include <map>
#include <memory>
#include <string>
#if __cplusplus > 201402L
    #include <shared_mutex> // for c++17
#else
    #include <shared_mutex_kes.h>   // for c++11
#endif

#include <CSource.h>
#include <Enum_common.h>

template <typename ADDR_TYPE, typename _Compare = std::less<ADDR_TYPE>> 
class CAliasAddr {
private:
    using AddressMapType = std::map<std::string /*alias*/, CSource /*essential-addr*/>;
    using AliasMapType = std::map<ADDR_TYPE /*address*/, std::string /*alias*/, _Compare>;

public:
    using AddrIterator = AddressMapType::iterator;
    using AliasIterator = typename AliasMapType::iterator;

public:
    CAliasAddr(void);

    ~CAliasAddr(void);

    AddrIterator begin(void) { return map_addr.begin(); };

    AddrIterator end(void) { return map_addr.end(); };

    bool is_there(std::string alias);

    bool is_there(const ADDR_TYPE &addr);

    std::shared_ptr<ADDR_TYPE> get(std::string alias);

    const ADDR_TYPE * get_read_only(std::string alias);

    std::string get(const ADDR_TYPE &addr);

    bool insert(std::string alias, 
                std::shared_ptr<ADDR_TYPE> &address, 
                enum_c::ProviderType pvd_type, 
                bool &is_new);

    void remove(std::string alias);

    void remove(ADDR_TYPE &addr);

    void clear(void);

private:
    void remove_impl(std::string alias, ADDR_TYPE addr);

private:
    AddressMapType map_addr;

    AliasMapType map_alias;

    std::shared_mutex mtx_sync;

};

#endif // _C_ALIAS_ADDRESS_H_