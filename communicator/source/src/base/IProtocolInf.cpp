#include <cassert>

#include <logger.h>
#include <IProtocolInf.h>

template bool IProtocolInf::set_property<int>(const std::string key, int value);
template bool IProtocolInf::set_property<long>(const std::string key, long value);
template bool IProtocolInf::set_property<float>(const std::string key, float value);
template bool IProtocolInf::set_property<double>(const std::string key, double value);


/******************************
 * Public Function Definition
 */ 
IProtocolInf::IProtocolInf(void): payload::CPayload() {
    segments.clear();
}

IProtocolInf::IProtocolInf(std::string name): payload::CPayload(name) {
    segments.clear();
}

IProtocolInf::~IProtocolInf(void) {
    LOGD("Called.");
    segments.clear();
}

std::shared_ptr<std::list<std::string>> IProtocolInf::get_keys(void) {
    std::shared_ptr<std::list<std::string>> ret;
    LOGERR("undefined function.");
    return ret;
}

std::string IProtocolInf::get_property(const std::string key) {
    std::string ret;
    LOGERR("undefined function.");
    return ret;
}

template <typename T>
bool IProtocolInf::set_property(const std::string key, T value) {
    return set_property_raw(key, std::to_string(value) );
}

template <>
bool IProtocolInf::set_property(const std::string key, const char* value) {
    return set_property_raw(key, value);
}

template <>
bool IProtocolInf::set_property(const std::string key, std::string value) {
    return set_property_raw(key, value);
}

/*******************************
 * Protected Function Definition
 */
IProtocolInf::SegmentsType& IProtocolInf::pack_recursive(const void* msg, size_t msg_size, enum_c::ServerType server_type) {
    LOGD("Called");
    auto proto_chain = get_proto_chain();
    assert(proto_chain->end() != proto_chain->begin());
    bool res = false;
    #define GET_PROTOCOL(iterator)  (*(iterator))->get(payload::CPayload::Myself_Name)

    try {
        auto itr = proto_chain->begin();
        auto pre_protocol = GET_PROTOCOL(itr);
        assert( (res = pre_protocol->pack(msg, msg_size, server_type)) == true );

        if ( (*itr)->get_name() !=  payload::CPayload::Default_Name ) {
            for (itr++; itr != proto_chain->end(); itr++) {
                // Current-protocol packing processing is start.
                SegmentsType& u_segments = pre_protocol->get_segments();
                SegmentsType::iterator itor;

                for( itor=u_segments.begin(); itor != u_segments.end(); itor++ ) {
                    RawMsgType& segment = *itor;
                    res = GET_PROTOCOL(itr)->pack(segment->get_msg_read_only(), segment->get_msg_size(), server_type);
                    assert(res == true);
                }
                pre_protocol.reset();
                pre_protocol = GET_PROTOCOL(itr);
            }
        }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        get_segments().clear();
    }

    return get_segments();
}

bool IProtocolInf::unpack_recurcive(const void* msg_raw, size_t msg_size) {
    LOGD("Called");
    auto proto_chain = get_proto_chain();
    assert(proto_chain->end() != proto_chain->begin());
    bool res = false;
    #define GET_PROTOCOL(iterator)  (*(iterator))->get(payload::CPayload::Myself_Name)

    try{
        auto itr = proto_chain->end();
        itr--;
        auto pre_protocol = GET_PROTOCOL(itr);
        assert( (res = pre_protocol->unpack(msg_raw, msg_size)) == true );
        assert( (res = !pre_protocol->is_empty()) == true );
        
        MsgType payload = pre_protocol->get_payload();
        if ( (*itr)->get_name() !=  payload::CPayload::Default_Name ) {
            for (; itr != proto_chain->begin();) {
                itr--;
                res = GET_PROTOCOL(itr)->unpack(payload->get_msg_read_only(), 
                                                payload->get_msg_size());
                assert(res == true);
                pre_protocol.reset();
                pre_protocol = GET_PROTOCOL(itr);
            }
        }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        res = false;
    }

    return res;
}

IProtocolInf::SegmentsType& IProtocolInf::get_segments(void) { 
    return segments; 
}

// fragment message. & make some segments.
bool IProtocolInf::pack(const void* msg_raw, size_t msg_size, enum_c::ServerType server_type) {
    LOGI("Dumy protocol for Empty or NULL desp_protocol.json file.");
    assert(msg_raw != NULL);
    assert(msg_size > 0);
    bool res = false;
    
    try {
        // Make one-segment & regist the segment to segment-list.
        std::shared_ptr<SegmentType> one_segment = std::make_shared<SegmentType>();
        one_segment->set_new_msg(msg_raw, msg_size);
        get_segments().push_back(one_segment);
        res = true;
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
    
    return res;
}

// classify segment. & extract payloads. & combine payloads. => make One-payload.
bool IProtocolInf::unpack(const void* msg_raw, size_t msg_size) {
    LOGI("Dumy protocol for Empty or NULL desp_protocol.json file.");
    assert(msg_raw != NULL);
    assert(msg_size > 0);
    bool res = false;
    
    try {
        // Set classified_data of msg_raw to payload.
        get_payload()->set_new_msg(msg_raw, msg_size);
        res = true;
    }
    catch ( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return res;
}

bool IProtocolInf::set_property_raw(const std::string key, const std::string value) {
    LOGERR("undefined function.");
    return false;
}