/***
 * CPayload.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
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
: _name_(name), _flag_op_(0) {
    _payload_ = std::make_shared<DataType>();
    _protocol_chain_name_.clear();
    _protocol_chain_.reset();
}

CPayload::~CPayload(void) {
    LOGD("Called.");
    _protocol_chain_.reset();
    _protocol_chain_name_.clear();
    _name_.clear();
    _payload_.reset();
    _flag_op_ = 0;
}

const std::string CPayload::get_name(void) {
    return this->_name_;
}

std::shared_ptr<IProtocolInf> CPayload::get(std::string proto_name) {
    assert(proto_name.empty() == false);
    assert(proto_name.length() > 0);

    if (proto_name == Myself_Name) {
        return get_protocol();
    }

    try{
        PayloadType target;
        auto proto_chain = get_proto_chain();
        ProtoChainType::iterator itr = proto_chain->begin();

        if ( proto_chain->size() <= 0 ) {
            throw CException(E_ERROR::E_INVALID_MEMBER_VARIABLES);
        }

        // Search to find protocol-name that is equal with input proto_name.
        for (; itr != proto_chain->end(); itr++) {
            target.reset();
            target = (*itr);
            if ( target->get_name() == proto_name ) {
                break;
            }
        }

        if ( itr == proto_chain->end() ) {
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

/** Get Flag with regard to Payload-Operating. */
CPayload::FlagDataType CPayload::get_op_flag(E_PAYLOAD_FLAG target) {
    if ( target == E_PAYLOAD_FLAG::E_NONE ) {
        return _flag_op_;
    }
    return (_flag_op_ & target);
}

/** Set Flag with regard to Payload-Operating. */
void CPayload::set_op_flag(E_PAYLOAD_FLAG target, bool value) {
    assert( target != E_PAYLOAD_FLAG::E_NONE );

    if ( value == false ) {
        _flag_op_ = _flag_op_ & (~target);
    }
    else {
        _flag_op_ = _flag_op_ | target;
    }
}


/************************************
 * Protected Function Definition
 */
inline std::shared_ptr<IProtocolInf> CPayload::get_protocol(void) {
    PayloadType myself = SharedThisType::shared_from_this();
    return std::dynamic_pointer_cast<IProtocolInf>( myself );
}

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

std::shared_ptr<CPayload::ProtoChainType> CPayload::get_proto_chain(void) {
    assert(_protocol_chain_name_.empty() == false) ;

    if ( auto proto_chain = _protocol_chain_.lock() ) { 
    	return proto_chain;
    }
    else {
        throw CException(E_ERROR::E_INVALID_MEMBER_VARIABLES);
    }
}

void CPayload::set_proto_chain(std::string chain_name, std::shared_ptr<ProtoChainType>& proto_chain) {
    assert(chain_name.empty() == false);
    assert(proto_chain.get() != NULL);
    _protocol_chain_ = proto_chain;
    _protocol_chain_name_ = chain_name;
}

std::string CPayload::get_protocols_chain_name(void) {
    return _protocol_chain_name_;
}


}   // namespace payload
