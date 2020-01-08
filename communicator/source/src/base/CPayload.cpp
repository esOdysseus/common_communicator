#include <cassert>

#include <logger.h>
#include <CPayload.h>
#include <IProtocolInf.h>

namespace payload
{

static const char* exception_switch(E_ERROR err_num) {
    switch(err_num) {
    case E_ERROR::E_NO_ERROR:
        return "E_NO_ERROR in payload pkg.";
    case E_ERROR::E_INVALID_MEMBER_VARIABLES:
        return "E_INVALID_MEMBER_VARIABLES in payload pkg.";
    case E_ERROR::E_ITS_NOT_SUPPORTED_TYPE:
        return "E_ITS_NOT_SUPPORTED_TYPE in payload pkg.";
    case E_ERROR::E_INVALID_VALUE:
        return "E_INVALID_VALUE in payload pkg.";
    default:
        return "\'not support error_type\' in payload pkg.";
    }
}
#include <CException.h>


/************************************
 * Public Function Definition
 */
CPayload::CPayload(std::string name) 
: _name_(name) {
    _payload_ = std::make_shared<DataType>();
    _protocol_chain_.reset();
}

CPayload::~CPayload(void) {
    if (_protocol_chain_.get() != NULL) {
        // Delete a element in list that is myself CPayload.
        auto itr = _protocol_chain_->begin();
        for (; itr != _protocol_chain_->end(); itr++) {
            if ( (*itr)->get_name() == _name_ ) {
                break;
            }
        }

        if ( itr != _protocol_chain_->end() ) {
            _protocol_chain_->erase(itr);
        }
    }
    _protocol_chain_.reset();
    _name_.clear();
    _payload_.reset();
}

const std::string CPayload::get_name(void) {
    return this->_name_;
}

std::shared_ptr<IProtocolInf> CPayload::get(std::string proto_name) {
    try{
        assert(proto_name.empty() == false);
        assert(proto_name.length() > 0);
        PayloadType target;
        ProtoChainType::iterator itr = _protocol_chain_->begin();

        if ( _protocol_chain_->size() <= 0 ) {
            throw CException(E_ERROR::E_INVALID_MEMBER_VARIABLES);
        }

        // Search to find protocol-name that is equal with input proto_name.
        for (; itr != _protocol_chain_->end(); itr++) {
            target.reset();
            target = (*itr);
            if ( target->get_name() == proto_name ) {
                break;
            }
        }

        if ( itr == _protocol_chain_->end() ) {
            throw CException(E_ERROR::E_ITS_NOT_SUPPORTED_TYPE);
        }

        return std::dynamic_pointer_cast<IProtocolInf>( target );
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }
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

bool CPayload::is_empty(void) {
    LOGD("It's called.");
    return (_payload_->get_msg_size() > 0) ? false : true;
}

/************************************
 * Protected Function Definition
 */
std::shared_ptr<CPayload::DataType> CPayload::get_payload(void) { 
    return _payload_; 
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

void CPayload::set_proto_chain(std::shared_ptr<ProtoChainType>& proto_chain) {
    assert(proto_chain.get() != NULL);
    _protocol_chain_ = proto_chain;
}


}   // namespace payload