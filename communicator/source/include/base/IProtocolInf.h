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
    using RawMsgType = std::shared_ptr<CRawMessage>;
    using MsgType = std::shared_ptr<payload::CPayload::DataType>;
    using SegmentType = CRawMessage;
    using SegmentsType = std::list<std::shared_ptr<SegmentType>>;
    using PropertyMap = std::map<std::string, std::string>;

public:
    IProtocolInf(void);

    IProtocolInf(std::string name);

    ~IProtocolInf(void);

    std::shared_ptr<PropertyMap> get_property(void);

// protected:
    SegmentsType& pack_recursive(const void* msg, size_t msg_size, enum_c::ServerType server_type);

    bool unpack_recurcive(const void* msg_raw, size_t msg_size);

    SegmentsType& get_segments(void);

    // fragment message. & make some segments.
    virtual bool pack(const void* msg_raw, size_t msg_size, enum_c::ServerType server_type);

    // classify segment. & extract payloads. & combine payloads. => make One-payload.
    virtual bool unpack(const void* msg_raw, size_t msg_size);

    virtual bool set_property(const std::shared_ptr<PropertyMap> &properties);

protected:
    SegmentsType segments;   // packed messages.

    std::shared_ptr<PropertyMap> properties;    // Description of properties.

    std::shared_ptr<std::list<IProtocolInf>> proto_chain;   // link to external list-object.

};


#endif // INTERFACE_PROTOCOL_H_`