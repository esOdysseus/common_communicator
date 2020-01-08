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
    using SegmentType = CRawMessage;
    using SegmentsType = std::list<std::shared_ptr<SegmentType>>;
    using ProtoChainType = payload::CPayload::ProtoChainType;

protected:
    using MsgType = std::shared_ptr<payload::CPayload::DataType>;

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

protected:
    SegmentsType& pack_recursive(const void* msg, size_t msg_size, enum_c::ServerType server_type);

    bool unpack_recurcive(const void* msg_raw, size_t msg_size);

    // fragment message. & make some segments.
    virtual bool pack(const void* msg_raw, size_t msg_size, enum_c::ServerType server_type);

    // classify segment. & extract payloads. & combine payloads. => make One-payload.
    virtual bool unpack(const void* msg_raw, size_t msg_size);

    virtual bool set_property_raw(const std::string key, const std::string value);

    SegmentsType& get_segments(void);

    friend class IHProtocolInf;

private:
    SegmentsType segments;   // packed messages.

};


#endif // INTERFACE_PROTOCOL_H_