#include <cassert>
#include <string.h>
#include <memory>

#include <logger.h>
#include <protocol/CPBigEndian.h>

#define MAKE_RAW_DATA(dest, src, seq)	((uint8_t*)(dest))[(seq)] = (src).big_endian.raw_##seq
#define MAKE_PROTOCOL(dest, src, seq)	(dest).big_endian.raw_##seq = ((const uint8_t*)(src))[(seq)]


/********************************
 * Public Function Definition
 */
CPBigEndian::CPBigEndian(void) 
: IProtocolInf(Protocol_NAME) {
    clear();
}

CPBigEndian::~CPBigEndian(void) {
    clear();
}

void CPBigEndian::clear(void) {
    protocol.header.msg_id = NULL;
    protocol.header.length = NULL;
}

std::shared_ptr<std::list<std::string>> CPBigEndian::get_keys(void) {
    using KeysType = std::list<std::string>;
    std::shared_ptr<KeysType> ret = std::make_shared<KeysType>();

    ret->push_back(MSG_ID);
    ret->push_back(LENGTH);
    return ret;
}

std::string CPBigEndian::get_property(const std::string key) {
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
bool CPBigEndian::pack(const void* msg_raw, size_t msg_size, enum_c::ServerType server_type) {
    LOGD("It's called.");

    size_t msg_length = 0;
    auto lamda_get_length = [&](void) -> size_t {
        size_t length = 0;

        if (server_type == enum_c::ServerType::E_SERVER_UDP) {
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

bool CPBigEndian::unpack(const void* msg_raw, size_t msg_size) {
    LOGD("It's called.");

    // Assumption : segments is only has one-segment.
    try{
        // Make classified_data & protocol-data
        const void* classified_data = unpack_raw_data(msg_raw, msg_size);
        assert(classified_data != NULL);

        // Set classified_data of msg_raw to payload.
        get_payload()->set_new_msg(classified_data, protocol.header.length);
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

bool CPBigEndian::set_property_raw(const std::string key, const std::string value) {
    bool ret = false;

    if (key.c_str() == MSG_ID) {
        set_msg_id( (MsgID_Type)(atoi(value.c_str())) );
        ret = true;
    }
    else if(key.c_str() == LENGTH) {
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
CPBigEndian::MsgID_Type CPBigEndian::get_msg_id(void) {
    return protocol.header.msg_id;
}

CPBigEndian::Length_Type CPBigEndian::get_length(void) {
    return protocol.header.length;
}

void CPBigEndian::set_msg_id(CPBigEndian::MsgID_Type value) {
    protocol.header.msg_id = value;
}

void CPBigEndian::set_length(CPBigEndian::Length_Type value) {
    protocol.header.length = value;
}

bool CPBigEndian::pack_raw_data(const void* msg_raw, size_t msg_size, UnitData_Type* raw_data, size_t raw_size) {
    assert( msg_raw != NULL );
    assert(0 < msg_size && msg_size < Segment_ByteSize);
    assert( raw_data != NULL && raw_size >= msg_size + sizeof(protocol) );

    try {
        UnitData_Type* src_buf = (UnitData_Type*)msg_raw;
        if (protocol.header.length == 0) {
            protocol.header.length = msg_size;
        }
        assert(protocol.header.length <= msg_size);

        MAKE_RAW_DATA(raw_data, protocol, 0);
        MAKE_RAW_DATA(raw_data, protocol, 1);
        MAKE_RAW_DATA(raw_data, protocol, 2);
        MAKE_RAW_DATA(raw_data, protocol, 3);
        MAKE_RAW_DATA(raw_data, protocol, 4);
        MAKE_RAW_DATA(raw_data, protocol, 5);
        MAKE_RAW_DATA(raw_data, protocol, 6);
        MAKE_RAW_DATA(raw_data, protocol, 7);

        memcpy(raw_data+sizeof(protocol), src_buf, protocol.header.length);\
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

const void* CPBigEndian::unpack_raw_data(const void* msg_raw, size_t msg_size) {
    assert( msg_raw != NULL && msg_size > sizeof(protocol) );

    try {
        MAKE_PROTOCOL(protocol, msg_raw, 0);
        MAKE_PROTOCOL(protocol, msg_raw, 1);
        MAKE_PROTOCOL(protocol, msg_raw, 2);
        MAKE_PROTOCOL(protocol, msg_raw, 3);
        MAKE_PROTOCOL(protocol, msg_raw, 4);
        MAKE_PROTOCOL(protocol, msg_raw, 5);
        MAKE_PROTOCOL(protocol, msg_raw, 6);
        MAKE_PROTOCOL(protocol, msg_raw, 7);

        assert( protocol.header.length <= (msg_size - sizeof(protocol)) );
        return ( ((const uint8_t*)msg_raw)+sizeof(protocol) );
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }
    return NULL;
}

