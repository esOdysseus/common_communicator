#ifndef C_PROTOCOL_LITTLE_ENDIAN_H_
#define C_PROTOCOL_LITTLE_ENDIAN_H_

#include <IProtocolInf.h>

class CPLittleEndian : public IProtocolInf {
public:
    using MsgID_Type = uint32_t;
    using Length_Type = uint32_t;
    using UnitData_Type = uint8_t;
    static constexpr char* Protocol_NAME = "CPLittleEndian";
    static constexpr char* MSG_ID = "msg_id";
    static constexpr char* LENGTH = "length";

    using UProtocol = union UProtocol {
        UnitData_Type little_endian[8];

        struct header {
            MsgID_Type msg_id 	: 32;
            Length_Type length	: 32;
        } header;
    };

public:
    CPLittleEndian(void);

    ~CPLittleEndian(void);

    void clear(void);

    std::shared_ptr<std::list<std::string>> get_keys(void) override;

    std::string get_property(const std::string key) override;

protected:
    bool pack(const void* msg_raw, size_t msg_size, enum_c::ProviderType provider_type) override;

    bool unpack(const void* msg_raw, size_t msg_size) override;

    bool set_property_raw(const std::string key, const std::string value) override;

private:
    MsgID_Type get_msg_id(void);

    Length_Type get_length(void);

    void set_msg_id(MsgID_Type value);

    void set_length(Length_Type value);

    const void* unpack_raw_data(const void* msg_raw, size_t msg_size);

    bool pack_raw_data(const void* msg_raw, size_t msg_size, 
                       UnitData_Type* raw_data, size_t raw_size);

private:
    UProtocol protocol;

    static constexpr unsigned int Segment_ByteSize = 1500U;     // MTU size

};

#endif // C_PROTOCOL_LITTLE_ENDIAN_H_