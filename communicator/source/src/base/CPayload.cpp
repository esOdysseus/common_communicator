#include <cassert>

#include <logger.h>
#include <CPayload.h>

namespace payload
{

static const char* exception_switch(E_ERROR err_num) {
    switch(err_num) {
    case E_ERROR::E_NO_ERROR:
        return "E_NO_ERROR in payload pkg.";
    case E_ERROR::E_ITS_NOT_SUPPORTED_TYPE:
        return "E_ITS_NOT_SUPPORTED_TYPE in payload pkg.";
    case E_ERROR::E_INVALID_VALUE:
        return "E_INVALID_VALUE in payload pkg.";
    default:
        return "\'not support error_type\' in payload pkg.";
    }
}
#include <CException.h>


CPayload::CPayload(std::string name) 
: next(NULL), _name_(name) {
    _payload_ = std::make_shared<DataType>();
}

CPayload::~CPayload(void) {
    _name_.clear();
    _payload_.reset();
    next.reset();
}

std::shared_ptr<CPayload> CPayload::get(std::string proto_name) {
    try{
        assert(proto_name.empty() == false);
        assert(proto_name.length() > 0);
        PayloadType target = SharedThisType::shared_from_this();

        while (target->get_name() != proto_name) {
            if (target->get_next().get() != NULL) {
                auto next_target = target->get_next();
                target.reset();
                target = next_target;
            }
            else{
                throw CException(E_ERROR::E_ITS_NOT_SUPPORTED_TYPE);
            }
        }

        return target;
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }
}

// template <typename PROTOCOL>
// std::shared_ptr<PROTOCOL> CPayload::get(void) {
//     try{
//         PayloadType target = SharedThisType::shared_from_this();
//         std::shared_ptr<PROTOCOL> sample = std::make_shared<PROTOCOL>(); 

//         while (target->get_name() != sample->get_name()) {
//             if (target->get_next().get() != NULL) {
//                 auto next_target = target->get_next();
//                 target.reset();
//                 target = next_target;
//             }
//             else{
//                 target.reset();
//                 break;
//             }
//         }

//         return std::dynamic_pointer_cast<PROTOCOL>(target);
//     }
//     catch(const std::exception &e) {
//         LOGERR("%s", e.what());
//         throw e;
//     }
// }

const std::string CPayload::get_name(void) {
    return this->_name_;
}

std::shared_ptr<CPayload::DataType> CPayload::get_payload(void) { 
    return _payload_; 
}

const void* CPayload::get_payload(size_t& payload_length) {
    return _payload_->get_msg_read_only(&payload_length);
}

bool CPayload::set_payload(const void* msg, size_t msg_size) {
    try{
        assert( msg != NULL && msg_size > 0 );
        _payload_->set_new_msg(msg, msg_size);
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

bool CPayload::set_payload(std::shared_ptr<CPayload::DataType>&& msg_raw) {
    try{
        _payload_.reset();
        _payload_ = std::forward<std::shared_ptr<DataType>>(msg_raw);
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

bool CPayload::is_there_data(void) {
    LOGD("It's called.");
    return (_payload_->get_msg_size() > 0) ? true : false;
}

CPayload::PayloadType CPayload::get_next(void) {
    return next;
}

void CPayload::insert_next(CPayload::PayloadType&& payload) {
    try{
        if (next.get() != NULL) {
            payload->insert_next( std::forward<PayloadType>(next) );
        }
        
        next = std::forward<PayloadType>(payload);
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }
}


}   // namespace payload