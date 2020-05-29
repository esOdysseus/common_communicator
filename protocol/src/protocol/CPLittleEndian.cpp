/***
 * CPLittleEndian.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <cassert>
#include <string.h>
#include <memory>

#include <logger.h>
#include <protocol/CPLittleEndian.h>

constexpr unsigned int CPLittleEndian::Segment_ByteSize;

/********************************
 * Public Function Definition
 */
CPLittleEndian::CPLittleEndian(void) 
: IProtocolInf(Protocol_NAME) {
    clear();
}

CPLittleEndian::~CPLittleEndian(void) {
    clear();
}

void CPLittleEndian::clear(void) {
    clean_head_tail();
}

std::shared_ptr<std::list<std::string>> CPLittleEndian::get_keys(void) {
    using KeysType = std::list<std::string>;
    std::shared_ptr<KeysType> ret = std::make_shared<KeysType>();

    ret->push_back(MSG_ID);
    ret->push_back(LENGTH);
    return ret;
}

std::string CPLittleEndian::get_property(const std::string key) {
    std::string ret;

    if (key == MSG_ID) {
        ret = std::to_string(get_msg_id());
    }
    else if(key == LENGTH) {
        ret = std::to_string(get_length());
    }
    else {
        LOGERR("Not supported key(%s).", key.c_str());
    }

    return ret;
}


/**********************************
 * Protected Function Definition
 */
bool CPLittleEndian::pack(const void* msg_raw, size_t msg_size, enum_c::ProviderType provider_type) {
    LOGD("It's called.");

    size_t msg_length = 0;
    auto lamda_get_length = [&](void) -> size_t {
        size_t length = 0;

        if (provider_type == enum_c::ProviderType::E_PVDT_TRANS_UDP) {
            // We only support single-segment.
            assert(msg_size < Segment_ByteSize);
            length = msg_size;
            msg_size = 0;
        }
        else {
            if ( msg_size >= Segment_ByteSize) {
                length = Segment_ByteSize;
                msg_size -= Segment_ByteSize;
            }
            else {
                length = msg_size;
                msg_size = 0;
            }
        }

        return length;
    };

    try {
        while( msg_size > 0 ) {
            msg_length = lamda_get_length();
            
            // Make raw-data from protocol + msg_raw.
            size_t raw_size = msg_length + sizeof(protocol);
            uint8_t raw_data[raw_size] = {0,};
            assert(pack_raw_data(msg_raw, msg_length, raw_data, raw_size) == true);

            // Make one-segment & regist the segment to segment-list.
            std::shared_ptr<SegmentType> one_segment = std::make_shared<SegmentType>();
            one_segment->set_new_msg(raw_data, raw_size);
            get_segments().push_back(one_segment);
        }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

bool CPLittleEndian::unpack(const void* msg_raw, size_t msg_size) {
    LOGD("It's called.");

    try{
        // Make classified_data & protocol-data
        const void* classified_data = unpack_raw_data(msg_raw, msg_size);
        assert(classified_data != NULL);

        // Set classified_data of msg_raw to payload.
        get_payload()->set_new_msg(classified_data, protocol.header.length);
        // (*properties.get())[ MSG_ID ] = std::to_string(protocol.header.msg_id);
        // (*properties.get())[ LENGTH ] = std::to_string(protocol.header.length);
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

void CPLittleEndian::clean_head_tail(void) {
    LOGD("It's called.");
    protocol.header.msg_id = (MsgID_Type)NULL;
    protocol.header.length = (Length_Type)NULL;
}

bool CPLittleEndian::set_property_raw(const std::string key, const std::string value) {
    bool ret = false;

    if (key == MSG_ID) {
        set_msg_id( (MsgID_Type)(atoi(value.c_str())) );
        ret = true;
    }
    else if(key == LENGTH) {
        set_length( (Length_Type)(atoi(value.c_str())) );
        ret = true;
    }
    else {
        LOGERR("Not supported key(%s).", key.c_str());
    }

    return ret;
}


/**********************************
 * Private Function Definition
 */ 
CPLittleEndian::MsgID_Type CPLittleEndian::get_msg_id(void) {
    return protocol.header.msg_id;
}

CPLittleEndian::Length_Type CPLittleEndian::get_length(void) {
    return protocol.header.length;
}

void CPLittleEndian::set_msg_id(CPLittleEndian::MsgID_Type value) {
    protocol.header.msg_id = value;
}

void CPLittleEndian::set_length(CPLittleEndian::Length_Type value) {
    protocol.header.length = value;
}

const void* CPLittleEndian::unpack_raw_data(const void* msg_raw, size_t msg_size) {
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

bool CPLittleEndian::pack_raw_data(const void* msg_raw, size_t msg_size, UnitData_Type* raw_data, size_t raw_size) {
    assert( msg_raw != NULL );
    assert(0 < msg_size && msg_size < Segment_ByteSize);
    assert( raw_data != NULL && raw_size >= msg_size + sizeof(protocol) );

    try {
        UnitData_Type* src_buf = (UnitData_Type*)msg_raw;
        protocol.header.length = msg_size;

        memcpy(raw_data, protocol.little_endian, sizeof(protocol));
        memcpy(raw_data+sizeof(protocol), src_buf, protocol.header.length);
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}