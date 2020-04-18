/***
 * CPBigEndian.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef C_PROTOCOL_BIG_ENDIAN_H_
#define C_PROTOCOL_BIG_ENDIAN_H_

#include <IProtocolInf.h>

class CPBigEndian : public IProtocolInf {
public:
    using MsgID_Type = uint32_t;
    using Length_Type = uint32_t;
    using UnitData_Type = uint8_t;
    static constexpr char* Protocol_NAME = "CPBigEndian";
    static constexpr char* MSG_ID = "msg_id";
    static constexpr char* LENGTH = "length";

    using UProtocol = union UProtocol {
        struct big_endian{
            UnitData_Type raw_3 : 8;
            UnitData_Type raw_2 : 8;
            UnitData_Type raw_1 : 8;
            UnitData_Type raw_0 : 8;
            UnitData_Type raw_7 : 8;
            UnitData_Type raw_6 : 8;
            UnitData_Type raw_5 : 8;
            UnitData_Type raw_4 : 8;
        } big_endian;

        struct header {
            MsgID_Type msg_id 	: 32;
            Length_Type length	: 32;
        } header;
    };

public:
    CPBigEndian(void);

    ~CPBigEndian(void);

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

#endif // C_PROTOCOL_BIG_ENDIAN_H_