#include <cassert>
#include <algorithm>
#include <memory>

#include <logger.h>
#include <CRawMessage.h>
#include <type_traits>

template bool CRawMessage::set_source(std::shared_ptr<struct sockaddr_in> addr, const char* addr_str);
template bool CRawMessage::set_source(std::shared_ptr<int> addr, const char* addr_str);

/***********************************************************
 * Definition member-function of CSource Class.
 * */
CRawMessage::CSource::CSource(void) {
    addr_type = EADDR_TYPE::E_SERVER_NOT_DEF;
    alias.clear();
}

CRawMessage::CSource::~CSource(void){
    try{
        if(addr_type != EADDR_TYPE::E_SERVER_NOT_DEF) {
            address.reset();
        }
        addr_type = EADDR_TYPE::E_SERVER_NOT_DEF;
        alias.clear();
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }
}

template <typename ADDR_TYPE> 
void CRawMessage::CSource::init(std::shared_ptr<ADDR_TYPE> addr, 
                                const char* alias, 
                                enum_c::ServerType server_type) {
    try {
        std::shared_ptr<void> dumy = std::static_pointer_cast<void>(addr);
        this->address = move(dumy);
        this->addr_type = server_type;
        this->alias = alias;
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }
}

template <typename ADDR_TYPE> 
std::shared_ptr<ADDR_TYPE> CRawMessage::CSource::get_address(void) {
    try{
        return std::static_pointer_cast<ADDR_TYPE>(address);
    }catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }
}

std::string CRawMessage::CSource::get_alias(void) {
    return alias;
}

CRawMessage::CSource::EADDR_TYPE CRawMessage::CSource::get_addr_type(void) {
    return addr_type;
}

/***********************************************************
 * Definition member-function of CRawMessage Class.
 * */
CRawMessage::CRawMessage(int sockfd, size_t capacity) { 
    clear();
    init(sockfd, capacity);
}

CRawMessage::~CRawMessage(void) {
    destroy();
}

bool CRawMessage::init(int sockfd, size_t capacity) {
    try {
        if (capacity == 0) {
            capacity = 2*capacity_bin;
        }
        this->capacity = capacity; 
        this->msg_size = 0;
        this->msg = new MsgDataType[capacity];
        assert( this->msg != NULL );
        this->socketfd = sockfd;
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

void CRawMessage::clear(void) {
    msg = NULL;
    msg_size = 0;
    capacity = 0;
    socketfd = 0;
}

void CRawMessage::destroy(void) {
    if (msg != NULL) {
        delete msg;
        msg = NULL;
    }
    msg_size = 0;
    capacity = 0;
    socketfd = 0;
}

int CRawMessage::get_socket_fd(void) {
    return socketfd;
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

        std::copy(msg, msg+msg_size, (MsgDataType*)buffer);
        return msg_size;
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
    }
    return 0;
}

const void* CRawMessage::get_msg_read_only(size_t* msg_size) {
    assert(msg != NULL);
    
    if (msg_size != NULL) {
        *msg_size = get_msg_size();
    }
    return (const void*)msg;
}

bool CRawMessage::set_new_msg(const void* buf, size_t msize) {
    MsgDataType* buffer = (MsgDataType*)buf;
    assert( buffer != NULL && msize > 0);

    try {
        {   // clean msg-memory.
            std::lock_guard<std::mutex> guard(mtx_copy);
            if(msg_size > 0) {
                assert( msg != NULL );
                msg_size = 0;
            }
        }

        // check msg-buf capacity for msize.
        if ( msize > (capacity-1) ) {
            assert( extend_capacity(msize + capacity_bin) == true );
        }

        // copy message-data to msg-buf.
        std::lock_guard<std::mutex> guard(mtx_copy);
        std::copy(buffer, buffer+msize, msg+msg_size);
        msg_size += msize;
        *(msg+msg_size) = '\0';     // append NULL for string.
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

bool CRawMessage::append_msg(void* buf, size_t msize) {
    MsgDataType* buffer = (MsgDataType*)buf;
    assert( buffer != NULL && msize > 0);

    try {
        if ( (msg_size + msize) > (capacity-1) ) {
            assert( extend_capacity(msize + capacity_bin) == true );
        }

        // copy message-data to msg-buf.
        std::lock_guard<std::mutex> guard(mtx_copy);
        std::copy(buffer, buffer+msize, msg+msg_size);
        msg_size += msize;
        *(msg+msg_size) = '\0';     // append NULL for string.
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

template <typename ADDR_TYPE>
bool CRawMessage::set_source(std::shared_ptr<ADDR_TYPE> addr, const char* addr_str) {
    try{
        enum_c::ServerType server_type = policy_addr<ADDR_TYPE>();
        source.init(addr,addr_str, server_type);
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

CRawMessage::LanAddrType CRawMessage::get_source_addr(std::string& alias, enum_c::ServerType server_type) {
    try {
        assert( source.get_addr_type() == enum_c::ServerType::E_SERVER_NOT_DEF || 
                source.get_addr_type() == server_type );
        alias = source.get_alias();
        return source.get_address<struct sockaddr_in>();
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }
}

CRawMessage::LanSockType CRawMessage::get_source_sock(std::string& alias, enum_c::ServerType server_type) {
    try {
        assert( source.get_addr_type() == enum_c::ServerType::E_SERVER_NOT_DEF || 
                source.get_addr_type() == server_type );
        alias = source.get_alias();
        return source.get_address<int>();
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }
}

const struct sockaddr_in* CRawMessage::get_source_addr_read_only(enum_c::ServerType server_type) {
    std::string src_name;
    LanAddrType src = get_source_addr(src_name, server_type);
    LOGD("source-Name=%s", src_name.c_str());
    return (const struct sockaddr_in*)src.get();
}

std::string CRawMessage::get_source_alias(void) {
    return source.get_alias();
}

template <typename ADDR_TYPE>
enum_c::ServerType CRawMessage::policy_addr(void) {
    try{
        if( std::is_same<ADDR_TYPE, struct sockaddr_in>::value == true ) {
            return enum_c::ServerType::E_SERVER_UDP;
        }
        else if( std::is_same<ADDR_TYPE, int>::value == true ) {            // int Socket
            return enum_c::ServerType::E_SERVER_TCP;
        }
        else {
            throw std::invalid_argument("Not Supported Address-Type.");
        }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }

    return enum_c::ServerType::E_SERVER_NOT_DEF;
}

bool CRawMessage::extend_capacity(size_t append_capacity) {
    assert( append_capacity > 0 );

    try {
        MsgDataType* new_msg = new MsgDataType[capacity + append_capacity];
        assert( new_msg != NULL );

        std::lock_guard<std::mutex> guard(mtx_copy);
        std::copy(msg, msg+msg_size, new_msg);
        delete msg;
        msg = new_msg;
        capacity += append_capacity;
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

