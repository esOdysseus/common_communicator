/***
 * CPUniversalCMD.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef C_PROTOCOL_UNIVERSAL_COMMAND_H_
#define C_PROTOCOL_UNIVERSAL_COMMAND_H_

#include <IProtocolInf.h>
#include <functional>

#define SOF_SIZE      4
#define CHAR_SIZE     19
#define TOTAL_SIZE    (SOF_SIZE + 22 + (2*CHAR_SIZE))

class CPUniversalCMD : public IProtocolInf {
private:
    static constexpr const char* SOF = "UCMD";
    using Tgfunc_Type = std::function<std::string(void)>;
    using Tsfunc_Type = std::function<void(const std::string& /*value*/)>;

public:
    using MsgID_Type = uint32_t;
    using State_Type = uint32_t;
    using When_Type = double;
    using Length_Type = uint32_t;
    using UnitData_Type = uint8_t;
    static constexpr const char* Protocol_NAME = "CPUniversalCMD";

    static constexpr const char* FLAG = "flag";
    static constexpr const char* STATE = "state";
    static constexpr const char* MSG_ID = "msg_id";
    static constexpr const char* FROM = "from";
    static constexpr const char* WHO = "who";
    static constexpr const char* WHEN = "when";
    static constexpr const char* LENGTH = "length";

    #pragma pack(push, 1)
    using UProtocol = union UProtocol {
        UnitData_Type little_endian[TOTAL_SIZE];

        struct header {
            char sof[SOF_SIZE];         // Start of Field ("UCMD")
            UnitData_Type flag  : 8;    // TypeList example: KEEP-ALIVE/RESP/ACK/ACTION-DONE/ERROR/REQUIRED-OPT
            uint32_t _reserved_ : 8;
            State_Type state    : 16;   // describe about Error/Event-State.
            MsgID_Type msg_id   : 32;   // TID of Request-UCMD.
            char from[CHAR_SIZE + 1];   // Send-APP name.
            char who[CHAR_SIZE + 1];    // Receive-APP name. (string)
            When_Type when;             // Sent-TIME by Send-APP for time-driven task-scheduling.
            Length_Type length;         // size of body. (for: where, what, how, why)
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
              std::string &&from_app, std::string &&to_app) override;

    bool unpack(const void* msg_raw, size_t msg_size) override;

    void clean_head_tail(void) override;

    bool set_property_raw(const std::string key, const std::string value) override;

private:
    // getter
    UnitData_Type get_flag(void);

    State_Type get_state(void);

    MsgID_Type get_msg_id(void);

    char* get_from(void);

    char* get_who(void);

    double get_when(void);

    Length_Type get_length(void);

    // setter
    void set_sof(void);

    void set_flag(UnitData_Type value);

    void set_state(State_Type value);

    void set_msg_id(MsgID_Type value);

    void set_from(const std::string value);

    void set_who(const std::string value);

    void set_when(void);

    void set_length(Length_Type value);

    // etc functions
    bool check_sof_validation(void);

    void set_getter_methods( void );

    void set_setter_methods( void );

    bool pack_raw_data(const void* msg_raw, size_t msg_size, 
                       std::shared_ptr<SegmentType> segment, size_t raw_size);

    const void* unpack_raw_data(const void* msg_raw, size_t msg_size);

private:
    UProtocol protocol;

    std::map<std::string /*key*/, Tgfunc_Type /*method*/> m_getter;

    std::map<std::string /*key*/, Tsfunc_Type /*method*/> m_setter;

};

#endif // C_PROTOCOL_UNIVERSAL_COMMAND_H_