/***
 * CRawMessage.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <cassert>
#include <algorithm>
#include <memory>
#include <mutex>

#include <logger.h>
#include <CRawMessage.h>
#include <type_traits>

template bool CRawMessage::set_source(std::shared_ptr<struct sockaddr_in> addr, std::shared_ptr<cf_alias::IAliasPVD> peer_alias);
template bool CRawMessage::set_source(std::shared_ptr<struct sockaddr_un> addr, std::shared_ptr<cf_alias::IAliasPVD> peer_alias);
template bool CRawMessage::set_source(std::shared_ptr<int> addr, std::shared_ptr<cf_alias::IAliasPVD> peer_alias);

template bool CRawMessage::set_source(std::shared_ptr<struct sockaddr_in> addr, const char* app_path, const char* pvd_path, CRawMessage::PVDType pvd_type);
template bool CRawMessage::set_source(std::shared_ptr<struct sockaddr_un> addr, const char* app_path, const char* pvd_path, CRawMessage::PVDType pvd_type);
template bool CRawMessage::set_source(std::shared_ptr<int> addr, const char* app_path, const char* pvd_path, CRawMessage::PVDType pvd_type);

/***********************************************************
 * Definition member-function of CRawMessage Class.
 * */
CRawMessage::CRawMessage(uint64_t capa) { 
    clear();
    init(capa);
}

CRawMessage::~CRawMessage(void) {
    destroy();
}

bool CRawMessage::init(uint64_t capa) {
    try {
        if (capa == DEF_CAP_SIZE) {
            capa = (uint64_t)(2*capacity_bin);
        }
        this->capacity = static_cast<size_t>(capa + 1); 
        this->msg_size = 0;
        LOGD("Memory-Allocation with capacity=%lu", capacity);
        this->_m_msg_ = new MsgDataType[capacity];
        assert( this->_m_msg_ != NULL );
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

void CRawMessage::clear(void) {
    _m_msg_ = NULL;
    msg_size = 0;
    capacity = 0;
}

void CRawMessage::destroy(void) {
    if (_m_msg_ != NULL) {
        delete [] _m_msg_;
        _m_msg_ = NULL;
    }
    msg_size = 0;
    capacity = 0;
}

size_t CRawMessage::get_msg_size(void) {
    return msg_size;
}

size_t CRawMessage::get_msg(void* buffer, size_t cap) {
    try {
        assert(buffer != NULL);
        if(cap < msg_size) {
            throw std::invalid_argument("insufficent capacity of buffer. Please, Re-try agin.");
        }

        std::shared_lock<std::shared_mutex> guard(mtx_sync);
        std::copy(_m_msg_, _m_msg_+msg_size, (MsgDataType*)buffer);
        return msg_size;
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
    }
    return 0;
}

const void* CRawMessage::get_msg_read_only(size_t* msg_size) {
    // assert(_m_msg_ != NULL);
    
    if (msg_size != NULL) {
        *msg_size = get_msg_size();
    }
    return (const void*)_m_msg_;
}

bool CRawMessage::set_new_msg(const void* buf, size_t msize) {
    MsgDataType* buffer = (MsgDataType*)buf;
    assert( buffer != NULL && msize > 0);

    try {
        {   // clean msg-memory.
            std::lock_guard<std::shared_mutex> guard(mtx_sync);
            if(msg_size > 0) {
                assert( _m_msg_ != NULL );
                msg_size = 0;
            }
        }

        // check msg-buf capacity for msize.
        if ( msize > (capacity-1) ) {
            assert( extend_capacity(msize + capacity_bin) == true );
        }

        // copy message-data to msg-buf.
        std::lock_guard<std::shared_mutex> guard(mtx_sync);
        std::copy(buffer, buffer+msize, _m_msg_+msg_size);
        msg_size += msize;
        *(_m_msg_+msg_size) = '\0';     // append NULL for string.
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

bool CRawMessage::set_msg_hook( TfuncType func, size_t msize ) {
    assert( func != NULL );
    assert( msize > 0 );
    bool res = false;

    try {
        if( msize > (capacity-1) ) {
            LOGERR("msize(%u) > capacity-1(%u)", msize, capacity-1);
            throw std::overflow_error("msize is overflowed.");
        }

        std::lock_guard<std::shared_mutex> guard(mtx_sync);
        res = func(_m_msg_);
        if( res == true ) {
            msg_size = msize;
            *(_m_msg_ + msg_size) = '\0';     // append NULL for string.
        }
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        res = false;
    }
    return res;
}

bool CRawMessage::append_msg(void* buf, size_t msize) {
    MsgDataType* buffer = (MsgDataType*)buf;
    assert( buffer != NULL && msize > 0);

    try {
        if ( (msg_size + msize) > (capacity-1) ) {
            assert( extend_capacity(msize + capacity_bin) == true );
        }

        // copy message-data to msg-buf.
        std::lock_guard<std::shared_mutex> guard(mtx_sync);
        std::copy(buffer, buffer+msize, _m_msg_+msg_size);
        msg_size += msize;
        *(_m_msg_+msg_size) = '\0';     // append NULL for string.
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

template <typename ADDR_TYPE>
bool CRawMessage::set_source(std::shared_ptr<ADDR_TYPE> addr, std::shared_ptr<cf_alias::IAliasPVD> peer_alias) {
    assert(peer_alias.get() != NULL);
    return set_source(addr, peer_alias->path_parent().data(), peer_alias->name().data(), peer_alias->type());
}

CRawMessage::LanAddrType CRawMessage::get_source_addr(std::string& app_path, std::string& pvd_path, enum_c::ProviderType provider_type) {
    try {
        assert( source.get_addr_type() == enum_c::ProviderType::E_PVDT_NOT_DEFINE || 
                source.get_addr_type() == provider_type );
        app_path = source.get_app_path();
        pvd_path = source.get_pvd_path();
        return source.get_address<struct sockaddr_in>();
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }
}

CRawMessage::LanSockType CRawMessage::get_source_sock(std::string& app_path, std::string& pvd_path, enum_c::ProviderType provider_type) {
    try {
        assert( source.get_addr_type() == enum_c::ProviderType::E_PVDT_NOT_DEFINE || 
                source.get_addr_type() == provider_type );
        app_path = source.get_app_path();
        pvd_path = source.get_pvd_path();
        return source.get_address<int>();
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }
}

const struct sockaddr_in* CRawMessage::get_source_addr_read_only(enum_c::ProviderType provider_type) {
    std::string app_path;
    std::string pvd_id;
    LanAddrType src = get_source_addr(app_path, pvd_id, provider_type);
    LOGD("alias=%s%s%s", app_path.data(), "/", pvd_id.data());
    return (const struct sockaddr_in*)src.get();
}

int CRawMessage::get_source_sock_read_only(enum_c::ProviderType provider_type) {
    std::string app_path;
    std::string pvd_id;
    LanSockType src = get_source_sock(app_path, pvd_id, provider_type);
    LOGD("alias=%s%s%s", app_path.data(), "/", pvd_id.data());
    return *(src.get());
}

std::string CRawMessage::get_source_app(void) {
    return source.get_app_path();
}

std::string CRawMessage::get_source_pvd(void) {
    return source.get_pvd_path();
}

template <typename ADDR_TYPE>
CRawMessage::PVDType CRawMessage::policy_addr(PVDType pvd_type) {
    try{
        if( std::is_same<ADDR_TYPE, struct sockaddr_in>::value == true ) {
            assert( pvd_type == PVDType::E_PVDT_TRANS_UDP );
            return pvd_type;
        }
        else if( std::is_same<ADDR_TYPE, struct sockaddr_un>::value == true ) {
            assert( pvd_type == PVDType::E_PVDT_TRANS_UDS_UDP );
            return pvd_type;
        }
        else if( std::is_same<ADDR_TYPE, int>::value == true ) {            // int Socket
            assert( pvd_type == PVDType::E_PVDT_TRANS_TCP || pvd_type == PVDType::E_PVDT_TRANS_UDS_TCP );
            return pvd_type;
        }
        else {
            throw std::invalid_argument("Not Supported Address-Type.");
        }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }

    return PVDType::E_PVDT_NOT_DEFINE;
}

bool CRawMessage::extend_capacity(size_t append_capacity) {
    assert( append_capacity > 0 );

    try {
        MsgDataType* new_msg = new MsgDataType[capacity + append_capacity];
        assert( new_msg != NULL );

        std::lock_guard<std::shared_mutex> guard(mtx_sync);
        std::copy(_m_msg_, _m_msg_+msg_size, new_msg);
        delete _m_msg_;
        _m_msg_ = new_msg;
        capacity += append_capacity;
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

template <typename ADDR_TYPE>
bool CRawMessage::set_source(std::shared_ptr<ADDR_TYPE> addr, const char* app_path, const char* pvd_path, PVDType pvd_type) {
    try{
        pvd_type = policy_addr<ADDR_TYPE>(pvd_type);
        source.init(addr, app_path, pvd_path, pvd_type);
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}
