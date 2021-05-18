/***
 * IProtocolInf.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef INTERFACE_PROTOCOL_H_
#define INTERFACE_PROTOCOL_H_

#include <list>
#include <map>
#include <memory>

#include <CRawMessage.h>
#include <CPayload.h>
#include <Enum_common.h>


class IProtocolInf : public payload::CPayload {
public:
    using SegmentType = CRawMessage;
    using SegmentsType = std::list<std::shared_ptr<SegmentType>>;

public:
    IProtocolInf(void);

    IProtocolInf(std::string name);

    ~IProtocolInf(void);

    /** Get Keys for properties. */
    virtual std::shared_ptr<std::list<std::string>> get_keys(void);

    /** Get Value for property-key. */
    virtual std::string get_property(const std::string key);

    /** Set Value for property-key. */
    template <typename T>
    bool set_property(const std::string key, T value);

    void clean_data(bool tx_data=true, bool rx_data=true);

protected:
    SegmentsType& pack_recursive(const void* msg, size_t msg_size, enum_c::ProviderType provider_type,
                                 std::string &&from_app, std::string &&to_app);

    bool unpack_recurcive(const void* msg_raw, size_t msg_size);

    // fragment message. & make some segments.
    virtual bool pack(const void* msg_raw, size_t msg_size, enum_c::ProviderType provider_type,
                      std::string &&from_app, std::string &&to_app);

    // classify segment. & extract payloads. & combine payloads. => make One-payload.
    virtual bool unpack(const void* msg_raw, size_t msg_size);

    // clean header & tail-data of protocol
    virtual void clean_head_tail(void);

    virtual bool set_property_raw(const std::string key, const std::string value);

    // need for receiving of multiple message.
    virtual size_t get_msg_size(const void* data, size_t data_size);

    SegmentsType& get_segments(void);

};


#endif // INTERFACE_PROTOCOL_H_
