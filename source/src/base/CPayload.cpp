#include <cassert>
#include <iostream>

#include <CPayload.h>
#include <protocol/CPBigEndian.h>
#include <protocol/CPLittleEndian.h>

using namespace std;

template std::shared_ptr<CPBigEndian> CPayload::get<CPBigEndian>(void);
template std::shared_ptr<CPLittleEndian> CPayload::get<CPLittleEndian>(void);

CPayload::CPayload(std::string name) 
: next(NULL), _name_(name) {
    _payload_ = std::make_shared<DataType>();
}

CPayload::~CPayload(void) {
    _name_.clear();
    _payload_.reset();
    next.reset();
}

template <typename PROTOCOL>
std::shared_ptr<PROTOCOL> CPayload::get(void) {
    try{
        PayloadType target = SharedThisType::shared_from_this();
        std::shared_ptr<PROTOCOL> sample = std::make_shared<PROTOCOL>(); 

        while (target->get_name() != sample->get_name()) {
            if (target->get_next().get() != NULL) {
                auto next_target = target->get_next();
                target.reset();
                target = next_target;
            }
            else{
                target.reset();
            }
        }

        return std::dynamic_pointer_cast<PROTOCOL>(target);
    }
    catch(const std::exception &e) {
        cout << "[Error] CPayload::get() : " << e.what() << endl;
        throw e;
    }
}

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
        cout << "[Error] CPayload::set_payload() : " << e.what() << endl;
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
        cout << "[Error] CPayload::set_payload() : " << e.what() << endl;
        return false;
    }
    return true;
}

bool CPayload::is_there_data(void) {
    cout << "CPayload::is_there() is called." << endl;
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
        cout << "[Error] CPayload::insert_next() : " << e.what() << endl;
    }
}
