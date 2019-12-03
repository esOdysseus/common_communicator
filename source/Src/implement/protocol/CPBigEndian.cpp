#include <cassert>
#include <iostream>
#include <string.h>

#include <protocol/CPBigEndian.h>

using namespace std;

#define MAKE_RAW_DATA(dest, src, seq)	((__uint8_t*)(dest))[(seq)] = (src).big_endian.raw_##seq
#define MAKE_PROTOCOL(dest, src, seq)	(dest).big_endian.raw_##seq = ((const __uint8_t*)(src))[(seq)]

static const char* Protocol_NAME = "CPBigEndian";

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

bool CPBigEndian::pack(const void* msg_raw, size_t msg_size, enum_c::ServerType server_type) {
    cout << "CPBigEndian::pack() is called." << endl;

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
            __uint8_t raw_data[raw_size] = {0,};
            assert(pack_raw_data(msg_raw, msg_length, raw_data, raw_size) == true);

            // Make one-segment & regist the segment to segment-list.
            std::shared_ptr<SegmentType> one_segment = std::make_shared<SegmentType>();
            one_segment->set_new_msg(raw_data, raw_size);
            segments.push_back(one_segment);
        }
    }
    catch(const std::exception &e) {
        cout << "[Error] CPBigEndian::pack() : " << e.what() << endl;
        return false;
    }
    return true;
}

bool CPBigEndian::unpack(const void* msg_raw, size_t msg_size) {
    cout << "CPBigEndian::unpack() is called." << endl;

    try{
        // Make classified_data & protocol-data
        const void* classified_data = unpack_raw_data(msg_raw, msg_size);
        assert(classified_data != NULL);

        // Set classified_data of msg_raw to payload.
        get_payload()->set_new_msg(classified_data, protocol.header.length);
    }
    catch(const std::exception &e) {
        cout << "[Error] CPBigEndian::unpack() : " << e.what() << endl;
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
        return ( ((const __uint8_t*)msg_raw)+sizeof(protocol) );
    }
    catch(const std::exception &e) {
        cout << "[Error] CPBigEndian::unpack_raw_data() : " << e.what() << endl;
    }
    return NULL;
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
        cout << "[Error] CPBigEndian::pack_raw_data() : " << e.what() << endl;
        return false;
    }
    return true;
}