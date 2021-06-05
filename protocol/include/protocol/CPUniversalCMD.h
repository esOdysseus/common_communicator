/***
 * CPUniversalCMD.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef C_PROTOCOL_UNIVERSAL_COMMAND_H_
#define C_PROTOCOL_UNIVERSAL_COMMAND_H_

#include <functional>

#include <IProtocolInf.h>
#include <CRawMessage.h>

#define TOTAL_HEADER_SIZE      (4 + 4 + 4 + 8 + 1 + 3)     // 24 byte

class CPUniversalCMD : public IProtocolInf {
private:
    using Tgfunc_Type = std::function<std::string(void)>;
    using Tsfunc_Type = std::function<void(const std::string& /*value*/)>;

public:
    using MsgID_Type = uint32_t;
    using State_Type = uint32_t;
    using When_Type = double;       // 8 byte
    using Length_Type = uint32_t;
    using UnitData_Type = uint8_t;
    static constexpr const char* Protocol_NAME = "CPUniversalCMD";

    static constexpr const char* FLAG = "flag";
    static constexpr const char* STATE = "state";
    static constexpr const char* MSG_ID = "msg_id";
    static constexpr const char* FROM = "from";
    static constexpr const char* WHEN = "when";
    static constexpr const char* LENGTH = "length";

    #pragma pack(push, 1)
    using UProtocol = union UProtocol {
        UnitData_Type little_endian[TOTAL_HEADER_SIZE];

        struct header {
            uint16_t        hsize       : 16;   // Header-Size: < 65535 byte
            State_Type      state       : 16;   // describe about Error/Event-State.
            MsgID_Type      msg_id      : 32;   // TID of Request-UCMD.
            Length_Type     payload_size;       // size of body-payload. (for: where, what, how, why)
            When_Type       when;               // Sent-TIME by Send-APP for time-driven task-scheduling.
            UnitData_Type   flag        : 8;    // TypeList example: KEEP-ALIVE/RESP/ACK/ACTION-DONE/ERROR/REQUIRED-OPT
            uint32_t        _reserve_   : 16;
            uint8_t         is_from     : 8;    // boolean value whether there is from-data. (true: yes, false: no-data)
        } header;
    };
    #pragma pack(pop)

public:
    CPUniversalCMD(void);

    ~CPUniversalCMD(void);

    void clear(void);

    std::shared_ptr<std::list<std::string>> get_keys(void) override;

    std::string get_property(const std::string key) override;

protected:
    bool pack(const void* msg_raw, size_t msg_size, enum_c::ProviderType provider_type,
              std::string &&from_app) override;

    bool unpack(const void* msg_raw, size_t msg_size) override;

    void clean_head_tail(void) override;

    bool set_property_raw(const std::string key, const std::string value) override;

private:
    // getter
    size_t get_header_size(void);

    UnitData_Type get_flag(void);

    State_Type get_state(void);

    MsgID_Type get_msg_id(void);

    const char* get_from(void);

    double get_when(void);

    Length_Type get_length(void);

    // setter
    size_t set_header_size(void);

    void set_flag(UnitData_Type value);

    void set_state(State_Type value);

    void set_msg_id(MsgID_Type value);

    void set_from(const std::string&& value);

    void set_when(void);

    void set_length(Length_Type value);

    // etc functions
    void set_getter_methods( void );

    void set_setter_methods( void );

    bool pack_raw_data(const void* msg_raw, size_t msg_size, 
                       std::shared_ptr<SegmentType> segment, size_t raw_size);

    const void* unpack_raw_data(const void* msg_raw, size_t msg_size);

private:
    UProtocol protocol;

    std::string m_from;       // Send-APP name.

    std::map<std::string /*key*/, Tgfunc_Type /*method*/> m_getter;

    std::map<std::string /*key*/, Tsfunc_Type /*method*/> m_setter;

};

#endif // C_PROTOCOL_UNIVERSAL_COMMAND_H_