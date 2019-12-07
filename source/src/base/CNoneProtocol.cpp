#include <logger.h>
#include <IProtocolInf.h>

CNoneProtocol::CNoneProtocol(void) : IProtocolInf() {};

CNoneProtocol::~CNoneProtocol(void) {};

bool CNoneProtocol::pack(const void* msg_raw, size_t msg_size, enum_c::ServerType server_type) { 
    try{
        // Make one-segment & regist the segment to segment-list.
        std::shared_ptr<SegmentType> one_segment = std::make_shared<SegmentType>();
        one_segment->set_new_msg(msg_raw, msg_size);
        segments.push_back(one_segment);
    }catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true; 
};

bool CNoneProtocol::unpack(const void* msg_raw, size_t msg_size) { 
    try{
        // Set classified_data of msg_raw to payload.
        get_payload()->set_new_msg(msg_raw, msg_size);
    }catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true; 
};
