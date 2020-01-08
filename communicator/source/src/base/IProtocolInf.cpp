
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
    bool res = false;
    
    try {
        // if (std::is_same<NEXT_PROTOCOL, CNoneProtocol>::value == false) {
        //     assert( this->get_next().get() != NULL );

        //     // Get next-protocol from Linked-list of This-protocol.
        //     std::shared_ptr<NEXT_PROTOCOL> next_protocol = std::dynamic_pointer_cast<NEXT_PROTOCOL>(this->get_next());
        //     // Recursive packing processing is start.
        //     SegmentsType& u_segments = next_protocol->pack_recursive(msg, msg_size, server_type);
            
        //     // Current-protocol packing processing is start.
        //     SegmentsType::iterator itor;
        //     for( itor=u_segments.begin(); itor != u_segments.end(); itor++ ) {
        //         RawMsgType& segment = *itor;
        //         res = pack(segment->get_msg_read_only(), segment->get_msg_size(), server_type);
        //         assert(res == true);
        //     }
        // }
        // else {
        //     assert( (res = pack(msg, msg_size, server_type)) == true );
        // }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        get_segments().clear();
    }

    return get_segments();
}

bool IProtocolInf::unpack_recurcive(const void* msg_raw, size_t msg_size) {
    bool res = false;

    try{
        // assert( (res = unpack(msg_raw, msg_size)) == true );
        // assert( (res = !is_empty()) == true );

        // MsgType payload = get_payload();
        // if (std::is_same<NEXT_PROTOCOL, CNoneProtocol>::value == false) {
        //     std::shared_ptr<NEXT_PROTOCOL> next_protocol = std::make_shared<NEXT_PROTOCOL>();
        //     res = next_protocol->unpack_recurcive(payload->get_msg_read_only(), 
        //                                           payload->get_msg_size());
        //     assert(res == true);
        //     payload.reset();
        //     payload = move(next_protocol->get_payload());
        // }
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
    LOGERR("undefined function.");
    return false;
}

// classify segment. & extract payloads. & combine payloads. => make One-payload.
bool IProtocolInf::unpack(const void* msg_raw, size_t msg_size) {
    LOGERR("undefined function.");
    return false;
}

bool IProtocolInf::set_property_raw(const std::string key, const std::string value) {
    LOGERR("undefined function.");
    return false;
}