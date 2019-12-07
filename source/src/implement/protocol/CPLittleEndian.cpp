#include <cassert>
#include <string.h>

#include <logger.h>
#include <protocol/CPLittleEndian.h>

using namespace std;

static const char* Protocol_NAME = "CPLittleEndian";

CPLittleEndian::CPLittleEndian(void) 
: IProtocolInf(Protocol_NAME) {
    clear();
}

CPLittleEndian::~CPLittleEndian(void) {
    clear();
}

void CPLittleEndian::clear(void) {
    protocol.header.msg_id = NULL;
    protocol.header.length = NULL;
}

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

bool CPLittleEndian::pack(const void* msg_raw, size_t msg_size, enum_c::ServerType server_type) {
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
            segments.push_back(one_segment);
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
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
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
        if (protocol.header.length == 0) {
            protocol.header.length = msg_size;
        }
        assert(protocol.header.length <= msg_size);

        memcpy(raw_data, protocol.little_endian, sizeof(protocol));
        memcpy(raw_data+sizeof(protocol), src_buf, protocol.header.length);
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}