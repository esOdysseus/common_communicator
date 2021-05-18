/***
 * CPUniversalCMD.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <assert_kes.h>
#include <string.h>
#include <memory>

#include <logger.h>
#include <protocol/CPUniversalCMD.h>
#include <time_kes.h>

constexpr const char* CPUniversalCMD::SOF;
constexpr const char* CPUniversalCMD::Protocol_NAME;

constexpr const char* CPUniversalCMD::FLAG;
constexpr const char* CPUniversalCMD::STATE;
constexpr const char* CPUniversalCMD::MSG_ID;
constexpr const char* CPUniversalCMD::FROM;
constexpr const char* CPUniversalCMD::WHO;
constexpr const char* CPUniversalCMD::WHEN;
constexpr const char* CPUniversalCMD::LENGTH;


/********************************
 * Public Function Definition
 */
CPUniversalCMD::CPUniversalCMD(void) 
: IProtocolInf(Protocol_NAME) {
    ASSERT( TOTAL_SIZE == sizeof(UProtocol::header), 
            LOGERR("TOTAL_SIZE=%u, sizeof(UProtocol::header)=%u is miss-matched.", TOTAL_SIZE, sizeof(UProtocol::header)) );

    clear();
    set_getter_methods();
    set_setter_methods();
}

CPUniversalCMD::~CPUniversalCMD(void) {
    clear();
}

void CPUniversalCMD::clear(void) {
    clean_head_tail();
}

std::shared_ptr<std::list<std::string>> CPUniversalCMD::get_keys(void) {
    using KeysType = std::list<std::string>;
    std::shared_ptr<KeysType> ret = std::make_shared<KeysType>();

    ret->push_back(FLAG);
    ret->push_back(STATE);
    ret->push_back(MSG_ID);
    ret->push_back(FROM);
    ret->push_back(WHO);
    ret->push_back(WHEN);
    ret->push_back(LENGTH);
    return ret;
}

std::string CPUniversalCMD::get_property(const std::string key) {
    if( m_getter.find( key ) == m_getter.end() ) {
        LOGERR("Not supported key(%s).", key.data());
        throw std::out_of_range("Not supported key.");
    }

    return m_getter[ key ]();
}


/**********************************
 * Protected Function Definition
 */
bool CPUniversalCMD::pack(const void* msg_raw, size_t msg_size, enum_c::ProviderType provider_type,
                          std::string &&from_app, std::string &&to_app) {
    LOGD("It's called.");

    try {
        // Make raw-data from protocol + msg_raw.
        size_t raw_size = msg_size + sizeof(protocol);

        // set protocol header
        set_sof();
        set_when();
        set_from(from_app);
        set_who(to_app);
        set_length(msg_size);

        // Make one-segment & regist the segment to segment-list.
        std::shared_ptr<SegmentType> one_segment = std::make_shared<SegmentType>(raw_size);
        assert(pack_raw_data(msg_raw, msg_size, one_segment, raw_size) == true);
        get_segments().push_back(one_segment);
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }

    return true;
}

bool CPUniversalCMD::unpack(const void* msg_raw, size_t msg_size) {
    LOGD("It's called.");

    try{
        // Make classified_data & protocol-data
        const void* classified_data = unpack_raw_data(msg_raw, msg_size);
        assert(classified_data != NULL);

        // Set classified_data of msg_raw to payload.
        get_payload()->set_new_msg(classified_data, get_length());
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    
    return true;
}

void CPUniversalCMD::clean_head_tail(void) {
    LOGD("It's called.");
    protocol.header.flag = (UnitData_Type)NULL;
    protocol.header.state = (State_Type)NULL;
    protocol.header.msg_id = (MsgID_Type)NULL;
    memset( protocol.header.from, 0, CHAR_SIZE+1 );
    memset( protocol.header.who, 0, CHAR_SIZE+1 );
    protocol.header.when = (When_Type)NULL;
    protocol.header.length = (Length_Type)NULL;

    // remove methods
    m_getter.clear();
    m_setter.clear();
}

bool CPUniversalCMD::set_property_raw(const std::string key, const std::string value) {
    if( m_setter.find( key ) == m_setter.end() ) {
        LOGW("Not supported key(%s).", key.data());
        return false;
    }

    m_setter[ key ](value);
    return true;
}


/**********************************
 * Private Function Definition
 */ 
// getter
CPUniversalCMD::UnitData_Type CPUniversalCMD::get_flag(void) {
    return protocol.header.flag;
}

CPUniversalCMD::State_Type CPUniversalCMD::get_state(void) {
    return protocol.header.state;
}

CPUniversalCMD::MsgID_Type CPUniversalCMD::get_msg_id(void) {
    return protocol.header.msg_id;
}

char* CPUniversalCMD::get_from(void) {
    return protocol.header.from;
}

char* CPUniversalCMD::get_who(void) {
    return protocol.header.who;
}

double CPUniversalCMD::get_when(void) {
    return protocol.header.when;
}

CPUniversalCMD::Length_Type CPUniversalCMD::get_length(void) {
    return protocol.header.length;
}

// setter
void CPUniversalCMD::set_sof(void) {
    Length_Type length = strlen(SOF);
    if ( length > SOF_SIZE ) {
        length = SOF_SIZE;
    }

    memcpy( protocol.header.sof, SOF, length );
}

void CPUniversalCMD::set_flag(UnitData_Type value) {
    protocol.header.flag = value;
}

void CPUniversalCMD::set_state(State_Type value) {
    protocol.header.state = value;
}

void CPUniversalCMD::set_msg_id(CPUniversalCMD::MsgID_Type value) {
    protocol.header.msg_id = value;
}

void CPUniversalCMD::set_from(const std::string value) {
    Length_Type length = CHAR_SIZE;
    if ( length > value.length() ) {
        length = value.length();
    }

    memcpy( protocol.header.from, value.data(), length );
    protocol.header.from[length+1] = 0;
}

void CPUniversalCMD::set_who(const std::string value) {
    Length_Type length = CHAR_SIZE;
    if ( length > value.length() ) {
        length = value.length();
    }

    memcpy( protocol.header.who, value.data(), length );
    protocol.header.who[length+1] = 0;
}

void CPUniversalCMD::set_when(void) {
    protocol.header.when = ::time_pkg::CTime::get<double>();
}

void CPUniversalCMD::set_length(CPUniversalCMD::Length_Type value) {
    protocol.header.length = value;
}

// etc functions.
bool CPUniversalCMD::check_sof_validation(void) {
    std::string str_sof;
    str_sof = protocol.header.sof[0];
    str_sof += protocol.header.sof[1];
    str_sof += protocol.header.sof[2];
    str_sof += protocol.header.sof[3];

    if( str_sof == SOF ) {
        return true;
    }
    return false;
}

void CPUniversalCMD::set_getter_methods( void ) {
    m_getter[FLAG] = [this](void) -> std::string {
        return std::to_string(get_flag());
    };

    m_getter[STATE] = [this](void) -> std::string {
        return std::to_string(get_state());
    };

    m_getter[MSG_ID] = [this](void) -> std::string {
        return std::to_string(get_msg_id());
    };

    m_getter[FROM] = [this](void) -> std::string {
        return std::string(get_from());
    };

    m_getter[WHO] = [this](void) -> std::string {
        return std::string(get_who());
    };

    m_getter[WHEN] = [this](void) -> std::string {
        return std::to_string(get_when());
    };

    m_getter[LENGTH] = [this](void) -> std::string {
        return std::to_string(get_length());
    };
}

void CPUniversalCMD::set_setter_methods( void ) {
    m_setter[FLAG] = [this](const std::string &value) -> void {
        set_flag( (UnitData_Type)( atoi(value.data()) ) );
    };

    m_setter[STATE] = [this](const std::string &value) -> void {
        set_state( (State_Type)( atoi(value.data()) ) );
    };

    m_setter[MSG_ID] = [this](const std::string &value) -> void {
        set_msg_id( (MsgID_Type)( atoi(value.data()) ) );
    };
}

bool CPUniversalCMD::pack_raw_data(const void* msg_raw, size_t msg_size, 
                                   std::shared_ptr<SegmentType> segment, size_t raw_size) {
    assert( segment.get() != NULL );
    assert( msg_raw != NULL && msg_size > 0 );
    assert( raw_size >= msg_size + sizeof(protocol) );

    try {
        UnitData_Type* src_buf = (UnitData_Type*)msg_raw;
        
        assert( protocol.header.flag != 0);
        assert( protocol.header.state != 0);
        assert( protocol.header.msg_id != 0);
        assert( protocol.header.from[0] != 0);
        assert( protocol.header.who[0] != 0);

        segment->set_msg_hook( [&](uint8_t* raw_data) -> bool {
                                memcpy( raw_data, protocol.little_endian, sizeof(protocol) );
                                memcpy( raw_data+sizeof(protocol), src_buf, get_length() );
                                return true;
                            }, raw_size );
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

const void* CPUniversalCMD::unpack_raw_data(const void* msg_raw, size_t msg_size) {
    assert( msg_raw != NULL && msg_size > sizeof(protocol) );

    try {
        memcpy(protocol.little_endian, msg_raw, sizeof(protocol));
        assert( protocol.header.length <= (msg_size - sizeof(protocol)) );
        return ( ((const uint8_t*)msg_raw)+sizeof(protocol) );
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }
    return NULL;
}
