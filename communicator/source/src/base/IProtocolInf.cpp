/***
 * IProtocolInf.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <cassert>

#include <logger.h>
#include <IProtocolInf.h>

#define GET_PROTOCOL(iterator)  (*(iterator))->get(payload::CPayload::Myself_Name)

template bool IProtocolInf::set_property<int8_t>(const std::string key, int8_t value);
template bool IProtocolInf::set_property<int16_t>(const std::string key, int16_t value);
template bool IProtocolInf::set_property<int32_t>(const std::string key, int32_t value);
template bool IProtocolInf::set_property<int64_t>(const std::string key, int64_t value);
template bool IProtocolInf::set_property<float>(const std::string key, float value);
template bool IProtocolInf::set_property<double>(const std::string key, double value);
template bool IProtocolInf::set_property<uint8_t>(const std::string key, uint8_t value);
template bool IProtocolInf::set_property<uint16_t>(const std::string key, uint16_t value);
template bool IProtocolInf::set_property<uint32_t>(const std::string key, uint32_t value);
template bool IProtocolInf::set_property<uint64_t>(const std::string key, uint64_t value);

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

void IProtocolInf::clean_data(bool tx_data, bool rx_data) {
    LOGD("Called");
    std::shared_ptr<IProtocolInf> protocol;
    auto proto_chain = get_proto_chain();

    try {
        for( auto itor=proto_chain->begin(); itor != proto_chain->end(); itor++ ) {
            protocol.reset();
            protocol = GET_PROTOCOL(itor);

            // clean data
            if( tx_data ) {
                protocol->get_segments().clear();   // clean segments for Tx.
            }

            if( rx_data ) {
                protocol->get_payload().reset();    // clean payload for Rx.
                protocol->clean_head_tail();
            }
        }
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw ;
    }
}

/*******************************
 * Protected Function Definition
 */
IProtocolInf::SegmentsType& IProtocolInf::pack_recursive(const void* msg, size_t msg_size, 
                                                         enum_c::ProviderType provider_type) {
    LOGD("Called");
    auto proto_chain = get_proto_chain();
    assert(proto_chain->end() != proto_chain->begin());
    bool res = false;

    try {
        auto itr = proto_chain->begin();
        auto pre_protocol = GET_PROTOCOL(itr);
        assert( (res = pre_protocol->pack(msg, msg_size, provider_type)) == true );

        if ( (*itr)->get_name() !=  payload::CPayload::Default_Name ) {
            for (itr++; itr != proto_chain->end(); itr++) {
                // Current-protocol packing processing is start.
                SegmentsType& u_segments = pre_protocol->get_segments();
                SegmentsType::iterator itor;

                for( itor=u_segments.begin(); itor != u_segments.end(); itor++ ) {
                    RawMsgType& segment = *itor;
                    res = GET_PROTOCOL(itr)->pack(segment->get_msg_read_only(), segment->get_msg_size(), provider_type);
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
    bool res = false;
    MsgType payload;
    std::shared_ptr<CPayload::ProtoChainType> proto_chain;
    assert(msg_raw != NULL);
    assert(msg_size > 0);

    try{
        proto_chain = get_proto_chain();
        assert(proto_chain->end() != proto_chain->begin());

        auto itr = proto_chain->end();
        itr--;
        auto pre_protocol = GET_PROTOCOL(itr);
        assert( (res = pre_protocol->unpack(msg_raw, msg_size)) == true );
        assert( (res = !pre_protocol->is_empty()) == true );
        
        payload = pre_protocol->get_payload();

        for (; itr != proto_chain->begin();) {
            itr--;
            if ( (*itr)->get_name() !=  payload::CPayload::Default_Name ) {
                pre_protocol.reset();
                pre_protocol = GET_PROTOCOL(itr);
                res = pre_protocol->unpack(payload->get_msg_read_only(), 
                                            payload->get_msg_size());
                assert(res == true);
                assert( (res = !pre_protocol->is_empty()) == true );
                
                payload = pre_protocol->get_payload();
            }
        }
    }
    catch(const std::length_error &e) {
        LOGW("%s", e.what());
        throw;
    }
    catch(const std::out_of_range &e) {
        LOGW("%s", e.what());
        throw;
    }
    catch(const std::invalid_argument &e) {
        LOGW("%s", e.what());
        throw;
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        res = false;
    }

    return res;
}

// fragment message. & make some segments.
bool IProtocolInf::pack(const void* msg_raw, size_t msg_size, enum_c::ProviderType provider_type) {
    LOGD("Dumy protocol for Empty or NULL desp_protocol.json file.");
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
    LOGD("Dumy protocol for Empty or NULL desp_protocol.json file.");
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

void IProtocolInf::clean_head_tail(void) {
    LOGD("Dumy protocol for Empty or NULL desp_protocol.json file.");
}

bool IProtocolInf::set_property_raw(const std::string key, const std::string value) {
    LOGERR("undefined function.");
    return false;
}

size_t IProtocolInf::get_msg_size(const void* data, size_t data_size) {
    LOGD("Dumy protocol for Empty or NULL desp_protocol.json file.");
    return data_size;
}

IProtocolInf::SegmentsType& IProtocolInf::get_segments(void) { 
    return segments; 
}

