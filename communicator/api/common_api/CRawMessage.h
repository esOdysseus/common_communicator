/***
 * CRawMessage.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef C_RAW_MESSAGE_H_
#define C_RAW_MESSAGE_H_

#include <memory>
#include <Enum_common.h>

class CRawMessage {
public:
    using LanAddrType = std::shared_ptr<struct sockaddr_in>;
    using LanSockType = std::shared_ptr<int>;

public:
    CRawMessage(size_t capacity=0);

    ~CRawMessage(void);

    void destroy(void);

    // With Regard to Message
    bool set_new_msg(const void* buf, size_t msize);

    bool append_msg(void* buf, size_t msize);

    size_t get_msg_size(void);

    size_t get_msg(void* buffer, size_t cap);

    const void* get_msg_read_only(size_t* msg_size=NULL);

    // With Regard to Source. (Client Address & Alias name)
    template <typename ADDR_TYPE>
    bool set_source(std::shared_ptr<ADDR_TYPE> addr, const char* alias);

    LanAddrType get_source_addr(std::string& alias, enum_c::ProviderType provider_type);

    LanSockType get_source_sock(std::string& alias, enum_c::ProviderType provider_type);

    const struct sockaddr_in* get_source_addr_read_only(enum_c::ProviderType provider_type);

    int get_source_sock_read_only(enum_c::ProviderType provider_type);

    std::string get_source_alias(void);

};

#endif // C_RAW_MESSAGE_H_
