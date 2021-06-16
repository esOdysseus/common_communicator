/***
 * CPVD_UDP.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <cassert>
#include <string.h>
#include <sys/stat.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <cassert>
#include <memory>

#include <logger.h>
#include <CRawMessage.h>
#include <provider/CPVD_UDP.h>
#include <provider/CHProtoBaseLan.h>

using namespace std::placeholders;

template class CPVD_UDP<struct sockaddr_in>;
template class CPVD_UDP<struct sockaddr_un>;

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

/******************************************
 * Definition of Public Function.
 */ 
template <>
CPVD_UDP<struct sockaddr_in>::CPVD_UDP(std::shared_ptr<cf_alias::IAliasPVD> self_alias, std::shared_ptr<cf_alias::CConfigAliases>& alia_manager, AliasPVDsType& alias_list)
: IPVDInf(self_alias, alia_manager), Cinet_uds(PF_INET, SOCK_DGRAM, AF_INET) {
    try{
        LOGD("Called.");
        _mm_ali4addr_.clear();
        assert( update_alias_mapper(alias_list) == true );
        _is_continue_ = false;
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }
}

template <>
CPVD_UDP<struct sockaddr_un>::CPVD_UDP(std::shared_ptr<cf_alias::IAliasPVD> self_alias, std::shared_ptr<cf_alias::CConfigAliases>& alia_manager, AliasPVDsType& alias_list)
: IPVDInf(self_alias, alia_manager), Cinet_uds(PF_FILE, SOCK_DGRAM, AF_UNIX) {
    try{
        LOGD("Called.");
        _mm_ali4addr_.clear();
        assert( update_alias_mapper(alias_list) == true );
        _is_continue_ = false;
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }
}

template <typename ADDR_TYPE>
CPVD_UDP<ADDR_TYPE>::~CPVD_UDP(void) {
    _is_continue_ = false;
    _mm_ali4addr_.clear();
    bzero(&servaddr, sizeof(servaddr));
}

template <typename ADDR_TYPE>
bool CPVD_UDP<ADDR_TYPE>::init(uint16_t port, std::string ip, ProviderMode mode) {
    /** UDP don't car about mode. Because, UDP has not classification of SERVER/CLIENT. */ 
    if (inited == true) {
        LOGERR("Already Init() is called. Please check it.");
        return inited;
    }

    // make UDP-Socket
    sockfd = Socket(1);
    if (sockfd < 0) {
        LOGERR("%d: %s", errno, strerror(errno));
        return false;
    }

    // activate Keep-Alive.
    // enable_keepalive(sockfd);

    // make serveraddr struct
    set_ip_port(servaddr, ip, port, ProviderMode::E_PVDM_SERVER);
    listeningPort = port;
    update_provider( get_pvd_alias(), ip, port );

    // tag flag
    started = false;
    inited = true;
    return inited;
}

template <typename ADDR_TYPE>
bool CPVD_UDP<ADDR_TYPE>::start(AppCallerType &app, std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager) {
    
    if (inited != true) {
        LOGERR("We need to init ServerTCP. Please check it.");
        return false;
    }

    // bind socket & server-address.
    if( Bind(sockfd, servaddr) < 0 ) {
        LOGERR("%d: %s", errno, strerror(errno));
        return false;
    }

    // Create instance of Protocol-Handler.
    assert( create_hprotocol<CHProtoBaseLan>(app, proto_manager) == true );

    started = true;
    return started;
}

template <typename ADDR_TYPE>
bool CPVD_UDP<ADDR_TYPE>::stop(void) {
    if (sockfd) {
        Close(sockfd);
        sockfd = 0;
    }

    return true;
}

template <typename ADDR_TYPE>
bool CPVD_UDP<ADDR_TYPE>::accept(void) {
    if(started) {
        std::shared_ptr<cf_alias::IAliasPVD> peer_alias;

        // sockfd is not client-socket-fd so, We will insert value-Zero(0) to parameter-2.
        _is_continue_ = true;
        if (thread_this_migrate(peer_alias, std::bind(&CPVD_UDP<ADDR_TYPE>::run_receiver, this, _1, _2), &_is_continue_) == false) {
            LOGERR("%d: %s", errno, strerror(errno));
            return false;
        }
        return true;
    }

    return false;
}

template <typename ADDR_TYPE>
int CPVD_UDP<ADDR_TYPE>::make_connection(std::string peer_full_path) {
    bool is_new = false;
    try{
        if( _mm_ali4addr_.is_there(peer_full_path) == true ) {
            auto addr = _mm_ali4addr_.get(peer_full_path);
            auto peer_alias = _mm_ali4addr_.get(*addr);
            assert( _mm_ali4addr_.insert(peer_alias, addr, is_new, true) == true );
            if( is_new == true ) {
                // trig connected call-back to app.
                hHprotocol->handle_connection(peer_alias->path_parent(), peer_alias->name(), true);
                assert( regist_connected_peer( peer_alias ) == true );
            }
            return true;
        }
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }
    return false;
}

template <typename ADDR_TYPE>
void CPVD_UDP<ADDR_TYPE>::disconnection(std::string app_path, std::string pvd_id) {
    bool had_connected = false;
    std::string peer_full_path = cf_alias::IAlias::make_full_path(app_path, pvd_id);

    try{
        if( _mm_ali4addr_.is_there(peer_full_path) ) {
            _mm_ali4addr_.reset_connect_flag(peer_full_path, had_connected);

            if ( had_connected ) {
                // trig connected call-back to app.
                hHprotocol->handle_connection(app_path, pvd_id, false);
            }
        }
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }
}

template <typename ADDR_TYPE>
typename CPVD_UDP<ADDR_TYPE>::MessageType CPVD_UDP<ADDR_TYPE>::read_msg(int u_sockfd, bool &is_new) {
    size_t msg_size = read_bufsize;
    
    u_sockfd = this->sockfd;
    assert(u_sockfd != 0 && read_buf != NULL && read_bufsize > 0);

    MessageType msg = std::make_shared<CRawMessage>();

    try {
        auto cliaddr = std::make_shared<ADDR_TYPE>();
        ADDR_TYPE * p_cliaddr = cliaddr.get();
        socklen_t clilen = sizeof( *p_cliaddr );
        std::string peer_full_path;
        std::shared_ptr<cf_alias::IAliasPVD> peer_alias;
        std::lock_guard<std::mutex> guard(mtx_read);

        while(msg_size == read_bufsize) {
            bzero(p_cliaddr, clilen);
            peer_alias.reset();

            // >>>> Return value description
            // -1 : Error.
            // >= 0 : The number of received message.
            // >>>> Flag Option description
            // MSG_OOB : Emergency-message receiving-mode such as out-of-band(X.25).
            // MSG_PEEK : Keep data in receive-queue of socket. & Read data from queue.
            // MSG_WAITALL : Wait until buffer is fulled.
            msg_size = recvfrom(u_sockfd, (char *)read_buf, read_bufsize,  
                                MSG_WAITALL, ( struct sockaddr *) p_cliaddr, &clilen); // Blocking Function.
            assert( msg_size >= 0 && msg_size <= read_bufsize);

            // Get client-ID
            peer_alias = make_client_id(*p_cliaddr);
            assert( peer_alias.get() != NULL );
            
            // Assumption : We will receive continuous-messages from identical-one-client.
            assert( peer_full_path.empty() == true || peer_full_path.compare(peer_alias->path()) == 0);
            peer_full_path = peer_alias->path();

            if( msg_size > 0 ) {
                assert(msg->append_msg(read_buf, msg_size) == true);
            }
        }

        assert( _mm_ali4addr_.insert(peer_alias, cliaddr, is_new, true) == true );
        msg->set_source(cliaddr, peer_alias);
        if( is_new == true ) {
            assert( regist_connected_peer( peer_alias ) == true );
        }
    }
    catch(const std::exception &e) {
        LOGERR("%d: %s", errno, strerror(errno));
        msg->destroy();
        throw e;
    }
    return msg;
}

template <typename ADDR_TYPE>
bool CPVD_UDP<ADDR_TYPE>::write_msg(std::string app_path, std::string pvd_path, MessageType msg) {
    assert( msg.get() != NULL );
    using RawDataType = CRawMessage::MsgDataType;

    int u_sockfd = this->sockfd;
    size_t msg_size = msg->get_msg_size();
    RawDataType* buffer = (RawDataType*)msg->get_msg_read_only();
    ADDR_TYPE* p_cliaddr = NULL;
    socklen_t clilen = sizeof( ADDR_TYPE );
    std::string peer_full_path;

    // soket check. if socket == 0, then we will copy default socket-number.
    assert(u_sockfd != 0 && buffer != NULL && msg_size > 0);

    // pvd_path is prepered. but, if pvd_path is null, then we will use pvd_path registed by msg.
    if ( pvd_path.empty() == true ) {
        app_path = msg->get_source_app();
        pvd_path = msg->get_source_pvd();
        if (pvd_path.empty() == true) {
            throw std::invalid_argument("pvd_path is NULL & msg doesn't have pvd_path. Please check it.");
        }
    }
    peer_full_path = cf_alias::IAlias::make_full_path(app_path, pvd_path);

    // if need trig about connection-call-back, then try it.
    if( make_connection(peer_full_path) == true ) {
        p_cliaddr = _mm_ali4addr_.get(peer_full_path).get();
    }
    else {
        std::string err_str = "peer_full_path("+peer_full_path+") is not pre-naming in provider-pkg.";
        throw std::out_of_range(err_str);
    }

    try {
        size_t written_size = 0;
        std::lock_guard<std::mutex> guard(mtx_write);

        while( msg_size > 0 ) {
            // return value description
            // -1 : Error.
            // >= 0 : The number of written message.
            // >>>> Flag Option description
            // MSG_OOB : Emergency-message receiving-mode such as out-of-band(X.25).
            // MSG_DONTROUTE : passing packet to Destination-address without gateway.
            // MSG_DONTWAIT : try non-blocking sending. (Error-Return value: EAGIN, EWOULDBLOCK)
            // MSG_MORE : there is additional packetes.
            // MSG_CONFIRM : It's available only when you use UDP or Raw socket.
            //             : It inform stat-data to Data-link layer that complete reply to client of current request.
            //             : It cause that Data-link layer need not to send ARP packet to client.
            written_size = sendto(u_sockfd, (const void*)buffer, msg_size,  
                                  MSG_CONFIRM, (const struct sockaddr *)p_cliaddr, 
                                  clilen); 
            assert(written_size > 0);

            msg_size -= written_size;
            buffer = buffer + written_size;
            written_size = 0;
        }

        return true;
    }
    catch(const std::exception &e) {
        LOGW("%s", e.what());
        LOGERR("%d: %s", errno, strerror(errno));
    }

    return false;    
}

/******************************************
 * Definition of Protected Function.
 */ 
template <typename ADDR_TYPE>
int CPVD_UDP<ADDR_TYPE>::enable_keepalive(int sock) {
    return 0;
}

template <typename ADDR_TYPE>
void CPVD_UDP<ADDR_TYPE>::update_alias_mapper(std::shared_ptr<cf_alias::IAliasPVD> new_pvd) {
    LOGD("Called.");
    bool is_new = false;
    std::string ip;
    uint16_t port_num = 0;
    std::shared_ptr<cf_alias::CAliasTrans> pvd_alias;
    std::shared_ptr<ADDR_TYPE> destaddr;

    try {
        pvd_alias = new_pvd->convert<cf_alias::CAliasTrans>( new_pvd );
        assert(pvd_alias.get() != NULL);
        destaddr = std::make_shared<ADDR_TYPE>();
        assert( pvd_alias->type() == get_provider_type());

        // make ADDR_TYPE variables.
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
bool CPVD_UDP<ADDR_TYPE>::update_alias_mapper(AliasPVDsType& alias_list) {
    LOGD("Called.");
    bool res = true;
    bool is_new = false;
    std::string ip;
    uint16_t port_num = 0;
    AliasPVDsType::iterator itor;
    std::string alias_name;
    std::shared_ptr<cf_alias::CAliasTrans> pvd_alias;
    std::shared_ptr<ADDR_TYPE> destaddr;
    
    try {
        for ( itor = alias_list.begin(); itor != alias_list.end(); itor++ ) {
            pvd_alias.reset();
            destaddr.reset();
            is_new = false;

            pvd_alias = (*itor)->convert<cf_alias::CAliasTrans>( *itor );
            assert(pvd_alias.get() != NULL);
            destaddr = std::make_shared<ADDR_TYPE>();
            assert( pvd_alias->type() == get_provider_type());

            // make ADDR_TYPE variables.
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
void CPVD_UDP<ADDR_TYPE>::run_receiver(std::shared_ptr<cf_alias::IAliasPVD> peer_alias, bool *is_continue) {
    if( peer_alias.get() == NULL ) {
        LOGI("Called without peer_alias(for All-Peers)");
    }
    else {
        LOGI("Called with peer_alias(%s)", peer_alias->path().data());
    }

    try {
        assert( is_continue != NULL );
        assert( hHprotocol->handle_initialization(get_provider_type(), true) == true );
        
        // Start receiver
        LOGD("Start MSG-receiver.");
        while(*is_continue) {
            bool is_new = false;
            MessageType msg_raw;

            // check received message 
            msg_raw = read_msg(0, is_new);     // get raw message. (Blocking)
            
            if( msg_raw->get_msg_size() > 0 ) {
                if( is_new == true ) {
                    // trig connected call-back to app.
                    hHprotocol->handle_connection(msg_raw->get_source_app(), msg_raw->get_source_pvd(), true);
                }
                // trig handling of protocol & Call-back to app
                assert( hHprotocol->handle_protocol_chain(msg_raw) == true );
            }
        }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        hHprotocol->handle_unintended_quit(e);
    }

    *is_continue = false;
}

template <typename ADDR_TYPE>
void CPVD_UDP<ADDR_TYPE>::disconnection(std::shared_ptr<cf_alias::IAliasPVD> peer_alias) {
    assert(peer_alias.get() != NULL);
    disconnection(peer_alias->path_parent(), peer_alias->name());
}

/******************************************
 * Definition of Private Function.
 */ 
template <typename ADDR_TYPE>
std::shared_ptr<cf_alias::IAliasPVD> CPVD_UDP<ADDR_TYPE>::make_new_client_alias(std::string pvd_id, const char* ip, const uint16_t port) {
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
std::shared_ptr<cf_alias::IAliasPVD> CPVD_UDP<ADDR_TYPE>::make_client_id(const ADDR_TYPE& cliaddr) {
    std::shared_ptr<cf_alias::IAliasPVD> peer;

    // Only support UDP/UDS_UDP M2M communication.
    assert( get_provider_type() == enum_c::ProviderType::E_PVDT_TRANS_UDP || 
            get_provider_type() == enum_c::ProviderType::E_PVDT_TRANS_UDS_UDP );

    try{
        if ( _mm_ali4addr_.is_there(cliaddr) == true ) {
            // Already exist address, then get alias in map.
            peer = _mm_ali4addr_.get(cliaddr);
        }
        else {
            // If unknown destination, then make new alias.
            bool is_new = false;
            std::string client_id;
            auto ipport = get_ip_port(cliaddr);

            if (strcmp(ipport->ip, "0.0.0.0") == 0) {
                throw std::out_of_range("Can not don't care 0.0.0.0 IP-peer.");
            }

            client_id = ipport->ip;
            client_id += ':' + std::to_string(ipport->port);
            peer = make_new_client_alias(client_id, ipport->ip, ipport->port);
            // append pair(pvd_alias & address) to mapper.
            _mm_ali4addr_.insert(peer, cliaddr, is_new);
        }
    }
    catch(const std::out_of_range &e) {
        LOGW("%s", e.what());
    }
    catch(const std::exception &e){
        LOGERR("%s", e.what());
    }

    return peer;
}

template <typename ADDR_TYPE>
void CPVD_UDP<ADDR_TYPE>::update_provider(std::shared_ptr<cf_alias::IAliasPVD> peer_alias, 
                                          std::string &ip, uint16_t &port ) {
    assert( peer_alias.get() != NULL );
    std::string pvd_type = peer_alias->convert(peer_alias->type());
    assert( pvd_type == cf_alias::IAliasPVD::UDP || pvd_type == cf_alias::IAliasPVD::UDP_UDS );

    try {
        auto pvd_trans = peer_alias->convert<cf_alias::CAliasTrans>( peer_alias );
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
