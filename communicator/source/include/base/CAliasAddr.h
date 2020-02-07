#ifndef _C_ALIAS_ADDRESS_H_
#define _C_ALIAS_ADDRESS_H_

#include <map>
#include <memory>
#include <string>

#include <CSource.h>
#include <Enum_common.h>

class CAliasAddr {
private:
    using AddressMapType = std::map<std::string /*alias*/, CSource /*essential-addr*/>;

public:
    using iterator = AddressMapType::iterator;

public:
    CAliasAddr(void);

    ~CAliasAddr(void);

    iterator begin(void) { return map_addr.begin(); };

    iterator end(void) { return map_addr.end(); };

    bool is_there(std::string alias);

    template <typename ADDR_TYPE> 
    std::shared_ptr<ADDR_TYPE> get(std::string alias);

    template <typename ADDR_TYPE> 
    const ADDR_TYPE * get_read_only(std::string alias);

    template <typename ADDR_TYPE> 
    bool insert(std::string alias, 
                std::shared_ptr<ADDR_TYPE> &address, 
                enum_c::ProviderType pvd_type, 
                bool &is_new);

    void remove(std::string alias);

    void clear(void);

private:
    AddressMapType map_addr;

};

#endif // _C_ALIAS_ADDRESS_H_