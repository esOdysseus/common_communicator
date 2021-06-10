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

constexpr const char* CPUniversalCMD::Protocol_NAME;

constexpr const char* CPUniversalCMD::FLAG;
constexpr const char* CPUniversalCMD::STATE;
constexpr const char* CPUniversalCMD::MSG_ID;
constexpr const char* CPUniversalCMD::FROM;
constexpr const char* CPUniversalCMD::WHEN;
constexpr const char* CPUniversalCMD::LENGTH;


/********************************
 * Public Function Definition
 */
CPUniversalCMD::CPUniversalCMD(void) 
: IProtocolInf(Protocol_NAME) {
    ASSERT( TOTAL_HEADER_SIZE == sizeof(UProtocol::header), 
            LOGERR("TOTAL_HEADER_SIZE=%u, sizeof(UProtocol::header)=%u is miss-matched.", TOTAL_HEADER_SIZE, sizeof(UProtocol::header)) );

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
                          std::string &&from_full_path) {
    LOGD("It's called.");

    try {
        // Make raw-data from protocol + msg_raw.
        size_t raw_size = 0;

        // set protocol header
        set_when();
        if( from_full_path.empty() == false ) {
            set_from(std::forward<std::string>(from_full_path));
        }
        set_length(msg_size);
        raw_size = msg_size + set_header_size();

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
    protocol.header.hsize = 0;
    protocol.header.flag = (UnitData_Type)NULL;
    protocol.header.state = (State_Type)NULL;
    protocol.header.msg_id = (MsgID_Type)NULL;
    protocol.header.when = (When_Type)NULL;
    protocol.header.payload_size = (Length_Type)NULL;
    protocol.header._reserve_ = 0;
    protocol.header.is_from = false;

    // Variable Protocol-field.
    m_from.clear();

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
size_t CPUniversalCMD::get_header_size(void) {
    return protocol.header.hsize;
}

CPUniversalCMD::UnitData_Type CPUniversalCMD::get_flag(void) {
    return protocol.header.flag;
}

CPUniversalCMD::State_Type CPUniversalCMD::get_state(void) {
    return protocol.header.state;
}

CPUniversalCMD::MsgID_Type CPUniversalCMD::get_msg_id(void) {
    return protocol.header.msg_id;
}

const char* CPUniversalCMD::get_from(void) {
    return m_from.data();
}

double CPUniversalCMD::get_when(void) {
    return protocol.header.when;
}

CPUniversalCMD::Length_Type CPUniversalCMD::get_length(void) {
    return protocol.header.payload_size;
}

// setter
size_t CPUniversalCMD::set_header_size(void) {
    protocol.header.hsize = sizeof(protocol);

    if( protocol.header.is_from == true ) {
        protocol.header.hsize += (m_from.length() + 1);
    }
    return protocol.header.hsize;
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

void CPUniversalCMD::set_from(const std::string&& value) {
    m_from = value;
    if( m_from.empty() != true ) {
        protocol.header.is_from = true;
    }
}

void CPUniversalCMD::set_when(void) {
    protocol.header.when = ::time_pkg::CTime::get<double>();
}

void CPUniversalCMD::set_length(CPUniversalCMD::Length_Type value) {
    protocol.header.payload_size = value;
}

// etc functions.
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
        return get_from();
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
    bool res = false;
    assert( segment.get() != NULL );
    // assert( msg_raw != NULL && msg_size > 0 );
    assert( raw_size >= msg_size + get_header_size() );

    try {
        UnitData_Type* src_buf = (UnitData_Type*)msg_raw;
        
        assert( protocol.header.flag != 0);
        assert( protocol.header.state != 0);
        assert( protocol.header.msg_id != 0);
        assert( get_length() == msg_size );

        res = segment->set_msg_hook( [&](uint8_t* raw_data) -> bool {
                                assert(raw_data != NULL);

                                memcpy( raw_data, protocol.little_endian, sizeof(protocol) );
                                if( protocol.header.is_from ) {
                                    memcpy( raw_data+sizeof(protocol), m_from.data(), m_from.length() );
                                    *(raw_data + sizeof(protocol) + m_from.length()) = 0;
                                }

                                if( get_length() > 0 ) {
                                    memcpy( raw_data+get_header_size(), src_buf, get_length() );
                                }
                                return true;
                            }, raw_size );
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        res = false;
    }

    return res;
}

const void* CPUniversalCMD::unpack_raw_data(const void* msg_raw, size_t msg_size) {
    assert( msg_raw != NULL && msg_size > sizeof(protocol) );

    try {
        memcpy(protocol.little_endian, msg_raw, sizeof(protocol));
        if( protocol.header.is_from ) {
            m_from = (const char*)( ((const uint8_t*)msg_raw) + sizeof(protocol) );
        }
        assert( protocol.header.payload_size <= (msg_size - get_header_size()) );
        return ( ((const uint8_t*)msg_raw) + get_header_size() );
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }
    return NULL;
}
