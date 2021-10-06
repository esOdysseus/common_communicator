/***
 * CPVD_TCP.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <iostream>
#include <cassert>
#include <string.h>
#include <sys/stat.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <errno.h>

#include <logger.h>
#include <CRawMessage.h>
#include <CConfigAliases.h>
#include <provider/CPVD_TCP.h>
#include <provider/CHProtoBaseLan.h>

using namespace std::placeholders;

template class CPVD_TCP<struct sockaddr_in>;
template class CPVD_TCP<struct sockaddr_un>;

// ***** Socket Type ****
// SOCK_STREAM :    TCP socket
// SOCK_DGRAM :     UDP socket
// SOCK_RAW :       No-wrapper socket
// **** Protocol Type ****
// PF_INET : IPv4
// PF_INET6 : IPv6
// PF_FILE : UNIX domain socket protocol
// PF_LOCAL : Local UNIX protocol
// PF_PACKET : Low-level-socket protocol
// PF_IPX : IPX novel protocol
// **** Address Type ****
// AF_INET : IPv4
// AF_INET6 : IPv6
// AF_UNIX  : UNIX domain socket address
// AF_LOCAL : Local UNIX address

typedef enum E_ERROR {
    E_TCP_NO_ERROR = 0,
    E_TCP_UNKNOWN_ALIAS = 1,
    E_TCP_HAVE_NOT_ALIAS = 2,
    E_TCP_NOT_SUPPORT_CLOUD_CONNECTION = 3,
    E_TCP_SESSION_CLOSED = 4,
    E_TCP_CREATE_THREAD_FAILED = 5,
    E_TCP_ACCEPT_FAILED = 6,
    E_TCP_CONNECT_FAILED = 7
}E_ERROR;

static const char* exception_switch(E_ERROR err_num) {
    switch(err_num) {
    case E_ERROR::E_TCP_NO_ERROR:
        return "E_TCP_NO_ERROR in provider pkg.";
    case E_ERROR::E_TCP_UNKNOWN_ALIAS:
        return "E_TCP_UNKNOWN_ALIAS in provider pkg.";
    case E_ERROR::E_TCP_HAVE_NOT_ALIAS:
        return "E_TCP_HAVE_NOT_ALIAS in provider pkg.";
    case E_ERROR::E_TCP_NOT_SUPPORT_CLOUD_CONNECTION:
        return "E_TCP_NOT_SUPPORT_CLOUD_CONNECTION in provider pkg.";
    case E_ERROR::E_TCP_SESSION_CLOSED:
        return "E_TCP_SESSION_CLOSED in provider pkg.";
    case E_ERROR::E_TCP_CREATE_THREAD_FAILED:
        return "E_TCP_CREATE_THREAD_FAILED in provider pkg.";
    case E_ERROR::E_TCP_ACCEPT_FAILED:
        return "E_TCP_ACCEPT_FAILED in provider pkg.";
    case E_ERROR::E_TCP_CONNECT_FAILED:
        return "E_TCP_CONNECT_FAILED in provider pkg.";
    default:
        return "\'not support error_type\' in provider pkg.";
    }
}

#include <CException.h>


template <>
CPVD_TCP<struct sockaddr_in>::CPVD_TCP(std::shared_ptr<cf_alias::IAliasPVD> self_alias, std::shared_ptr<cf_alias::CConfigAliases>& alia_manager, AliasPVDsType& alias_list)
: IPVDInf(self_alias, alia_manager), Cinet_uds(PF_INET, SOCK_STREAM, AF_INET) {
    try{
        _mode_ = ProviderMode::E_PVDM_NONE;
        _available_sockfd_ = 0;
        _mm_ali4sock_.clear();
        _mm_ali4addr_.clear();
        assert( update_alias_mapper(alias_list) == true );
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }
}

template <>
CPVD_TCP<struct sockaddr_un>::CPVD_TCP(std::shared_ptr<cf_alias::IAliasPVD> self_alias, std::shared_ptr<cf_alias::CConfigAliases>& alia_manager, AliasPVDsType& alias_list)
: IPVDInf(self_alias, alia_manager), Cinet_uds(PF_FILE, SOCK_STREAM, AF_UNIX) {
    try{
        _mode_ = ProviderMode::E_PVDM_NONE;
        _available_sockfd_ = 0;
        _mm_ali4sock_.clear();
        _mm_ali4addr_.clear();
        assert( update_alias_mapper(alias_list) == true );
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }
}

template <typename ADDR_TYPE>
CPVD_TCP<ADDR_TYPE>::~CPVD_TCP(void) {
    CAliasAddr<int>::AddrIterator itor;
    _mode_ = ProviderMode::E_PVDM_NONE;
    _available_sockfd_ = 0;
    
    for( itor = _mm_ali4sock_.begin(); itor != _mm_ali4sock_.end(); itor++ ) {
        int u_sockfd = *(_mm_ali4sock_.get(itor->first).get());
        thread_destroy(itor->first);
        usleep(10000);      // for wait thread delete-complete.
        Close(u_sockfd);
    }
    _mm_ali4sock_.clear();
    _mm_ali4addr_.clear();
    bzero(&servaddr, sizeof(servaddr));
    bzero(&cliaddr, sizeof(cliaddr));
}

template <typename ADDR_TYPE>
bool CPVD_TCP<ADDR_TYPE>::init(uint16_t port, std::string ip, ProviderMode mode) {
    if (inited == true) {
        LOGERR("Already Init() is called. Please check it.");
        return inited;
    }

    // make socket
    assert( (sockfd = make_socket(1)) > 0 );

    _mode_ = mode;
    switch(_mode_) {
    case ProviderMode::E_PVDM_BOTH:
    case ProviderMode::E_PVDM_SERVER:
        set_ip_port(servaddr, ip, port, _mode_);
        listeningPort = port;
        break;
    case ProviderMode::E_PVDM_CLIENT:
        set_ip_port(cliaddr, ip, port, _mode_);
        break;
    default:
        {
            std::string err_str = "Not supported ProviderMode(" + std::to_string((int)mode) +").";
            throw std::invalid_argument(err_str);
        }
    }

    // update Trans-provider
    update_provider( get_pvd_alias(), ip, port );

    // tag flag
    started = false;
    inited = true;

    return inited;
}

template <typename ADDR_TYPE>
bool CPVD_TCP<ADDR_TYPE>::start(AppCallerType &app, std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager) {
    started = false;

    if (inited != true) {
        LOGERR("We need to init ServerTCP. Please check it.");
        return started;
    }

    try {
        switch(_mode_) {
        case ProviderMode::E_PVDM_BOTH:
        case ProviderMode::E_PVDM_SERVER:
            // bind socket & server-address.
            if(Bind(sockfd, servaddr) < 0) {
                std::string err_str = "ServerAddr binding failed: "+ std::to_string(errno) +": "+ std::string(strerror(errno));
                throw std::runtime_error(err_str);
            }

            // start listen.
            if(Listen(sockfd, 5) < 0)
            {
                LOGERR("%d: Server : Can't listening connect: %s", errno, strerror(errno));
                return false;
            }
            break;
        case ProviderMode::E_PVDM_CLIENT:
            // bind socket & server-address.
            if(Bind(sockfd, cliaddr) < 0) {
                std::string err_str = "ClientAddr binding failed: "+ std::to_string(errno) +": "+ std::string(strerror(errno));
                throw std::runtime_error(err_str);
            }
            release_self_sockfd(sockfd);    // register sockfd for announce available-sockfd of client.
            break;
        default:
            {
                std::string err_str = "Not supported ProviderMode(" + std::to_string((int)_mode_) +").";
                throw std::invalid_argument(err_str);
            }
        }

        // Create instance of Protocol-Handler.
        assert( create_hprotocol<CHProtoBaseLan>(app, proto_manager) == true );
        assert( hHprotocol->handle_initialization(get_provider_type(), true) == true );

        started = true;
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }

    return started;
}

template <typename ADDR_TYPE>
bool CPVD_TCP<ADDR_TYPE>::stop(void) {
    if (sockfd) {
        Close(sockfd);
        sockfd = 0;
    }

    return true;
}

template <typename ADDR_TYPE>
bool CPVD_TCP<ADDR_TYPE>::accept(void) {
    try {
        if(started) {
            switch(_mode_) {
            case ProviderMode::E_PVDM_BOTH:
            case ProviderMode::E_PVDM_SERVER:
                server_accept();    // Blocking Function for Server.
                return true;
            case ProviderMode::E_PVDM_CLIENT:
                sleep(1);           // No Operation for Client.
                return true;
            default:
                {
                    std::string err_str = "Not supported ProviderMode(" + std::to_string((int)_mode_) +").";
                    throw std::invalid_argument(err_str);
                }
            }
        }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }
    
    return false;
}

template <typename ADDR_TYPE>
int CPVD_TCP<ADDR_TYPE>::make_connection(std::string peer_full_path) {
    bool is_new = false;
    int new_sockfd = -1;
    ADDR_TYPE *destaddr = NULL;
    std::shared_ptr<int> address = std::make_shared<int>();

    try{
        // get self socket on available.
        switch(_mode_) {
        case ProviderMode::E_PVDM_BOTH:
        case ProviderMode::E_PVDM_SERVER:
            assert( (new_sockfd = make_socket(1)) > 0 );
            if( get_provider_type() == enum_c::ProviderType::E_PVDT_TRANS_UDS_TCP ) {
                // for UDS client-socket setting.
                if( Bind_uds_client(new_sockfd) < 0 ) {
                    std::string err_str = "ClientAddr binding failed: "+ std::to_string(errno) +": "+ std::string(strerror(errno));
                    throw std::runtime_error(err_str);
                }
            }
            break;
        case ProviderMode::E_PVDM_CLIENT:
            new_sockfd = get_self_sockfd();
            if( new_sockfd == 0 ) {
                LOGW("Already, connected with another peer. Please check it.");
                return 0;
            }
            break;
        default:
            {
                std::string err_str = "Not supported ProviderMode(" + std::to_string((int)_mode_) +").";
                throw std::invalid_argument(err_str);
            }
        }

        // get destination address.
        if( _mm_ali4addr_.is_there(peer_full_path) == true ) {
            destaddr = (ADDR_TYPE*)_mm_ali4addr_.get(peer_full_path).get();
        }
        else {
            LOGERR("unknown alias --> %s", peer_full_path.data());
            throw CException(E_ERROR::E_TCP_UNKNOWN_ALIAS);
        }

        // if( connect(new_sockfd, (struct sockaddr *)destaddr, sizeof(*destaddr)) >= 0 ) {
        if( Connect(new_sockfd, *destaddr) >= 0 ) {
            LOGD("Connection to DEST.(%s) is success.", peer_full_path.data());
            auto peer_alias = _mm_ali4addr_.get( *destaddr );

            // register new_sockfd to _mm_ali4sock_.
            *(address.get()) = new_sockfd;
            assert( _mm_ali4sock_.insert(peer_alias, address, is_new, true) == true );
            assert( is_new == true );

            // create thread with PROTOCOL for new-sesseion by new-user.
            if (thread_create(peer_alias, std::bind(&CPVD_TCP::run_receiver, this, _1, _2)) == false) {
                LOGERR("%d: Thread Create failed: %s", errno, strerror(errno));
                disconnection(peer_alias);
                throw CException(E_ERROR::E_TCP_CREATE_THREAD_FAILED);
            }
        }
        else {
            LOGW("Connection to DEST.(%s) is failed.", peer_full_path.data());
            throw CException(E_ERROR::E_TCP_CONNECT_FAILED);
        }
    }
    catch(const CException &e) {
        switch(e.get_id()) {
        case E_ERROR::E_TCP_CONNECT_FAILED:
            LOGW("%s", e.what());
            break;
        default:
            LOGERR("%s", e.what());
        }

        if( _mode_ == ProviderMode::E_PVDM_CLIENT ) {
            release_self_sockfd(new_sockfd);
        } else {
            Close(new_sockfd);
        }
        new_sockfd = -1;
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        new_sockfd = -1;
    }

    return new_sockfd;
}

template <typename ADDR_TYPE>
void CPVD_TCP<ADDR_TYPE>::disconnection(std::string app_path, std::string pvd_id) {
    int u_sockfd = -1;
    std::string peer_full_path = cf_alias::IAlias::make_full_path(app_path, pvd_id);

    try {
        if ( _mm_ali4sock_.is_there(peer_full_path) == false) {
            LOGW("alias is not exist in _mm_ali4sock_ mapper.");
            return ;
        }
        u_sockfd = *(_mm_ali4sock_.get(peer_full_path).get());

        // need auto-lock
        std::lock_guard<std::mutex> guard(mtx_write);

        Close( u_sockfd );
        _mm_ali4sock_.remove(peer_full_path);
        if(_mode_ == ProviderMode::E_PVDM_CLIENT) {
            release_self_sockfd(u_sockfd);
        }
        unregist_connected_peer( peer_full_path );
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

template <typename ADDR_TYPE>
typename CPVD_TCP<ADDR_TYPE>::MessageType CPVD_TCP<ADDR_TYPE>::read_msg(int u_sockfd, bool &is_new) {
    ssize_t msg_size = read_bufsize;
    assert(u_sockfd > 0 && read_buf != NULL && read_bufsize > 0);
    MessageType msg = std::make_shared<CRawMessage>();
    is_new = false;

    try {
        while(msg_size == read_bufsize) {
            // return value description
            // 0 : End of Field.
            // -1 : Error. (session is Closed by peer)
            // > 0 : The number of received message.
            msg_size = read(u_sockfd, read_buf, read_bufsize);    // Blocking Function.
            if(0 > msg_size || msg_size > read_bufsize) {
                throw std::range_error("Range-Error about '0 <= msg_size && msg_size <= read_bufsize'.");
            }

            if( msg_size > 0 ){
                assert(msg->append_msg(read_buf, msg_size) == true);
            }
        }
    }
    catch(const std::range_error &e) {
        LOGW("%s", e.what());
        LOGW("errno:%d, msg:%s", errno, strerror(errno));
        msg->destroy();
        msg.reset();
        throw;
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        msg->destroy();
        msg.reset();
        throw e;
    }
    return msg;
}

template <typename ADDR_TYPE>
bool CPVD_TCP<ADDR_TYPE>::write_msg(std::string app_path, std::string pvd_path, MessageType msg) {
    assert( msg.get() != NULL );
    using RawDataType = CRawMessage::MsgDataType;

    bool res = true;
    int u_sockfd = -1;
    ssize_t written_size = 0;
    ssize_t msg_size = msg->get_msg_size();
    RawDataType* buffer = (RawDataType*)msg->get_msg_read_only();

    try {
        // alias is prepered. but, if alias is null, then we will use alias registed by msg.
        u_sockfd = get_connected_socket( cf_alias::IAlias::make_full_path(app_path, pvd_path), msg);
        assert(u_sockfd > 0 && buffer != NULL && msg_size > 0);
                
        std::lock_guard<std::mutex> guard(mtx_write);

        while( msg_size > 0 ) {
            // return value description
            // -1 : Error. (session is Closed by client)
            // > 0 : The number of written message.
            written_size=write(u_sockfd, (const void*)buffer, msg_size);
            if(written_size < 0) {
                LOGW("Session is closed by Client. MSG-Sending is failed.");
                res = false;
                break;
            }

            msg_size -= written_size;
            buffer = buffer + written_size;
            written_size = 0;
        }
    }
    catch(const std::exception &e) {
        LOGW("%s", e.what());
        LOGERR("%d: %s", errno, strerror(errno));
        res = false;
    }
    return res;
}

/************************************
 * Definition of Protected Function.
 */
template <typename ADDR_TYPE>
int CPVD_TCP<ADDR_TYPE>::enable_keepalive(int sock) {
    int yes = 1;

    if(setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int)) == -1) {
        std::cerr << errno << "  " << strerror(errno) << std::endl;
        return -1;
    }

    int idle = 1;

    if(setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int)) == -1) {
        std::cerr << errno << "  " << strerror(errno) << std::endl;
        return -1;
    }

    int interval = 1;

    if(setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int)) == -1) {
        std::cerr << errno << "  " << strerror(errno) << std::endl;
        return -1;
    }

    int maxpkt = 10;

    if(setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(int)) == -1) {
        std::cerr << errno << "  " << strerror(errno) << std::endl;
        return -1;
    }

    return 0;
}

template <typename ADDR_TYPE>
void CPVD_TCP<ADDR_TYPE>::update_alias_mapper(std::shared_ptr<cf_alias::IAliasPVD> new_pvd) {
    LOGD("Called.");
    bool is_new = false;
    std::string ip;
    uint16_t port_num = 0;
    std::shared_ptr<cf_alias::CAliasTrans> pvd_alias;
    std::shared_ptr<ADDR_TYPE> destaddr;
    
    try {
        // get pvd_alias info & alloc memory
        pvd_alias = new_pvd->convert<cf_alias::CAliasTrans>( new_pvd );
        assert(pvd_alias.get() != NULL);
        destaddr = std::make_shared<ADDR_TYPE>();
        assert( pvd_alias->type() == get_provider_type());

        // make sockaddr_in variables.
        ip = pvd_alias->get_ip();
        port_num = pvd_alias->get_port();
        set_ip_port(*destaddr.get(), ip, port_num, ProviderMode::E_PVDM_SERVER);

        // append pair(pvd_alias & address) to mapper.
        _mm_ali4addr_.insert(pvd_alias, destaddr, is_new);
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }
}

template <typename ADDR_TYPE>
bool CPVD_TCP<ADDR_TYPE>::update_alias_mapper(AliasPVDsType& alias_list) {
    LOGD("Called.");
    bool res = true;
    bool is_new = false;
    std::string ip;
    uint16_t port_num = 0;
    AliasPVDsType::iterator itor;
    std::shared_ptr<cf_alias::CAliasTrans> pvd_alias;
    std::shared_ptr<ADDR_TYPE> destaddr;

    try {
        for ( itor = alias_list.begin(); itor != alias_list.end(); itor++ ) {
            pvd_alias.reset();
            destaddr.reset();
            is_new = false;

            // get alias info & alloc memory
            pvd_alias = (*itor)->convert<cf_alias::CAliasTrans>( *itor );
            assert(pvd_alias.get() != NULL);
            destaddr = std::make_shared<ADDR_TYPE>();
            assert( pvd_alias->type() == get_provider_type());

            // make sockaddr_in variables.
            ip = pvd_alias->get_ip();
            port_num = pvd_alias->get_port();
            set_ip_port(*destaddr.get(), ip, port_num, ProviderMode::E_PVDM_SERVER);

            // append pair(pvd_alias & address) to mapper.
            _mm_ali4addr_.insert(pvd_alias, destaddr, is_new);
        }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        res = false;
        throw e;
    }

    return res;
}

template <typename ADDR_TYPE>
void CPVD_TCP<ADDR_TYPE>::run_receiver(std::shared_ptr<cf_alias::IAliasPVD> peer_alias, bool *is_continue) {
    assert(peer_alias.get() != NULL);
    LOGI("Called with alias(%s)", peer_alias->path().data());
    int socket_fd = -1;
    bool is_new = false;
    MessageType msg_raw;
    std::shared_ptr<int> socket = std::make_shared<int>();
    std::string peer_app = peer_alias->path_parent();
    std::string peer_pvd = peer_alias->name();
    std::string peer_full_path = peer_alias->path();

    auto decompose_full_path = [](std::string& full_path, std::string& app_path, std::string& pvd_id) -> bool {
        auto found = full_path.rfind("/");
        if( found != std::string::npos ) {
            app_path = full_path.substr(0, found);
            pvd_id = full_path.substr(found + 1, std::string::npos);

            LOGI("Full-Path(%s) is decomposed to %s , %s", full_path.data(), app_path.data(), pvd_id.data());
            return true;
        }
        return false;
    };

    auto lamda_update_peer_alias = std::make_shared<IHProtocolInf::TfuncUpdator>( [=](std::string&& full_path) mutable ->void {
        if( full_path.empty() == true || peer_full_path == full_path ) {
            return;
        }

        try {
            LOGW("Update Peer-Alias from %s, to %s", peer_full_path.data(), full_path.data());

            std::string new_app;
            std::string new_pvd;
            auto addr = _mm_ali4sock_.get(peer_full_path);
            // update peer_alias, peer_app, peer_pvd
            if( decompose_full_path( full_path, new_app, new_pvd ) == false ) {
                throw std::runtime_error("Failed decompose_full_path() function.");
            }

            // remove old peer-alias
            hHprotocol->handle_connection(peer_app, peer_pvd, false);
            unregist_connected_peer(peer_full_path);
            _mm_ali4sock_.remove(peer_full_path);

            // insert new peer-alias
            peer_full_path = full_path;
            peer_alias->update( new_app, new_pvd );
            _mm_ali4sock_.insert(peer_alias, addr, is_new, true);
            assert( regist_connected_peer( peer_alias ) == true );
            hHprotocol->handle_connection(new_app, new_pvd, true);
        }
        catch( const std::exception& e ) {
            LOGERR("%s", e.what());
        }
    });

    try {
        assert(is_continue != NULL && *is_continue == true);
        assert( _mm_ali4sock_.is_there(peer_full_path) == true );
        assert( (socket_fd = *(_mm_ali4sock_.get(peer_full_path).get())) > 0 );
        *(socket.get()) = socket_fd;

        // trig connected call-back to app.
        assert( regist_connected_peer( peer_alias ) == true );
        hHprotocol->handle_connection(peer_app, peer_pvd, true);

        // Start receiver
        LOGD("Start MSG-receiver.");
        while(*is_continue) {
            is_new = false;
            msg_raw.reset();

            try {
                // check received message 
                msg_raw = read_msg(socket_fd, is_new);     // get raw message. (Blocking)

                if( msg_raw->get_msg_size() > 0 ) {
                    msg_raw->set_source(socket, peer_alias); 
                    // trig handling of protocol & Call-back to app
                    assert( hHprotocol->handle_protocol_chain(msg_raw, lamda_update_peer_alias) == true );
                }
                else {
                    LOGW("msg_size == 0, we are regard this that connection close by peer.");
                    *is_continue = false;
                }
            }
            catch(const std::length_error &e) { // occure when payload length in packet is invalid.
                LOGW("%s", e.what());
            }
            catch(const std::out_of_range &e) { // occure when payload is NULL.
                LOGW("%s", e.what());
            }
            catch(const std::invalid_argument &e) { // occure when msg_size is invalid.
                LOGW("%s", e.what());
            }

            msg_raw.reset();
        }

        // trig connected call-back to app.
        hHprotocol->handle_connection(peer_alias->path_parent(), peer_alias->name(), false);    
    }
    catch(const std::range_error &e) {  // occure when connection close by peer.
        LOGW("%s", e.what());
        // trig connected call-back to app.
        hHprotocol->handle_connection(peer_alias->path_parent(), peer_alias->name(), false);
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        
        // trig connected call-back to app.
        hHprotocol->handle_connection(peer_alias->path_parent(), peer_alias->name(), false);
        hHprotocol->handle_unintended_quit(e);
    }

    *is_continue = false;
    disconnection(peer_alias);
    zombi_thread_migrate(peer_alias);
}

template <typename ADDR_TYPE>
void CPVD_TCP<ADDR_TYPE>::disconnection(std::shared_ptr<cf_alias::IAliasPVD> peer_alias) {
    assert(peer_alias.get() != NULL);
    disconnection(peer_alias->path_parent(), peer_alias->name());
}

/******************************************
 * Definition of Private Function.
 */ 
template <typename ADDR_TYPE>
int CPVD_TCP<ADDR_TYPE>::make_socket(int opt_flag) {
    int new_sockfd = -1;

    // make TCP-Socket
    if( (new_sockfd = Socket(opt_flag)) < 0 ) {
        LOGERR("%d: %s", errno, strerror(errno));
        return false;
    }

    // // activate Keep-Alive. (If UDS mode is enabled, then this code is unavailabled.)
    // enable_keepalive(new_sockfd);

    return new_sockfd;
}

template <typename ADDR_TYPE>
int CPVD_TCP<ADDR_TYPE>::get_connected_socket(std::string peer_full_path, MessageType &msg) {
    int u_sockfd = -1;

    try {
        // peer is prepered. but, if peer is null, then we will use peer registed by msg.
        if ( peer_full_path.empty() == false ) {
            if ( _mm_ali4sock_.is_there(peer_full_path) == true ) {
                u_sockfd = *(_mm_ali4sock_.get(peer_full_path).get());
            }
            else {
                u_sockfd=make_connection(peer_full_path);
                if( u_sockfd < 0 ) {
                    throw std::logic_error("make_connection failed.");
                }
            }
        }
        else {  // peer_full_path is NULL.
            if (msg->get_source_pvd().empty() == false) {
                u_sockfd = msg->get_source_sock_read_only(get_provider_type());
            }
            else {
                throw CException(E_ERROR::E_TCP_HAVE_NOT_ALIAS);
            }
        }
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        u_sockfd = -1;
        throw e;
    }

    return u_sockfd;
}

template <typename ADDR_TYPE>
std::shared_ptr<cf_alias::IAliasPVD> CPVD_TCP<ADDR_TYPE>::make_new_client_alias(std::string pvd_id, const char* ip, const uint16_t port) {
    std::shared_ptr<cf_alias::CAliasTrans> client;

    try {
        client = std::make_shared<cf_alias::CAliasTrans>( "", pvd_id.data(), cf_alias::IAliasPVD::convert(get_provider_type()).data() );
        assert(client.get() != NULL);

        client->set_ip(ip);
        client->set_port(port);
        client->set_mask(24);
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return client;
}

template <typename ADDR_TYPE>
std::shared_ptr<cf_alias::IAliasPVD> CPVD_TCP<ADDR_TYPE>::make_client_id(const ADDR_TYPE& new_cliaddr) {
    std::shared_ptr<cf_alias::IAliasPVD> cli_alias;

    // Only support TCP/UDS_TCP M2M communication.
    assert( get_provider_type() == enum_c::ProviderType::E_PVDT_TRANS_TCP || 
            get_provider_type() == enum_c::ProviderType::E_PVDT_TRANS_UDS_TCP );

    try{
        if ( _mm_ali4addr_.is_there(new_cliaddr) == true ) {
            // Already exist address, then get alias in map.
            cli_alias = _mm_ali4addr_.get(new_cliaddr);
        }
        else {
            // If unknown destination, then make new alias.
            bool is_new = false;
            std::string client_id;
            auto ipport = get_ip_port(new_cliaddr);

            if (strcmp(ipport->ip, "0.0.0.0") == 0) {
                throw std::out_of_range("Don't Care 0.0.0.0 IP peer.");
            }

            client_id = ipport->ip;
            client_id += ':' + std::to_string(ipport->port);
            cli_alias = make_new_client_alias(client_id, ipport->ip, ipport->port);
            // append pair(pvd_alias & address) to mapper.
            _mm_ali4addr_.insert(cli_alias, new_cliaddr, is_new);
        }
    }
    catch(const std::out_of_range &e) {
        LOGW("%s", e.what());
    }
    catch(const std::exception &e){
        LOGERR("%s", e.what());
    }

    return cli_alias;
}

template <typename ADDR_TYPE>
void CPVD_TCP<ADDR_TYPE>::server_accept(void) {
    bool is_new = false;
    ADDR_TYPE new_cliaddr;
    std::shared_ptr<cf_alias::IAliasPVD> peer_alias;
    std::shared_ptr<int> address = std::make_shared<int>();

    try{
        // start accept-blocking.
        int newsockfd = Accept(sockfd, new_cliaddr); // Blocking Function.
        if(newsockfd < 0) {
            LOGERR("%d: %s", errno, strerror(errno));
            throw CException(E_ERROR::E_TCP_ACCEPT_FAILED);
        }

        // Get client-ID
        peer_alias = make_client_id(new_cliaddr);
        if ( peer_alias.get() != NULL ) {
            *(address.get()) = newsockfd;
            assert( _mm_ali4sock_.insert(peer_alias, address, is_new, true) == true );
        
            // create thread with PROTOCOL for new-sesseion by new-user.
            if (thread_create(peer_alias, std::bind(&CPVD_TCP::run_receiver, this, _1, _2)) == false) {
                LOGERR("%d: Thread Create failed: %s", errno, strerror(errno));
                disconnection(peer_alias);
                throw CException(E_ERROR::E_TCP_CREATE_THREAD_FAILED);
            }
        }
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

template <typename ADDR_TYPE>
int CPVD_TCP<ADDR_TYPE>::get_self_sockfd(void) {              // for only Client-Mode.
    int temp_sockfd = _available_sockfd_;
    _available_sockfd_ = 0;
    return temp_sockfd;
}

template <typename ADDR_TYPE>
void CPVD_TCP<ADDR_TYPE>::release_self_sockfd(int release_sockfd) {    // for only Client-Mode.
    if (release_sockfd == sockfd) {
        _available_sockfd_ = sockfd;
    }
    else {
        LOGW("Now support sockfd-number(%d)", release_sockfd);
    }
}

template <typename ADDR_TYPE>
void CPVD_TCP<ADDR_TYPE>::update_provider(std::shared_ptr<cf_alias::IAliasPVD> pvd_alias, 
                                          std::string &ip, uint16_t &port ) {
    assert( pvd_alias.get() != NULL );
    std::string pvd_type = pvd_alias->convert(pvd_alias->type());
    assert( pvd_type == cf_alias::IAliasPVD::TCP || pvd_type == cf_alias::IAliasPVD::TCP_UDS );

    try {
        auto pvd_trans = pvd_alias->convert<cf_alias::CAliasTrans>( pvd_alias );
        std::string& pvd_ip = pvd_trans->get_ip_ref();
        uint32_t& pvd_port = pvd_trans->get_port_ref();

        if( pvd_ip.empty() == true || pvd_port == 0 ) {
            pvd_ip = ip;
            pvd_port = port;
            pvd_trans->set_mask(24);
        }
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}
