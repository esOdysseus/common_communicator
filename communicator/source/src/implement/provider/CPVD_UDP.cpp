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

// ***** Socket Type ****
// SOCK_STREAM :    TCP socket
// SOCK_DGRAM :     UDP socket
// SOCK_RAW :       No-wrapper socket
// **** Protocol Type ****
// PF_INET : IPv4
// PF_INET6 : IPv6
// PF_LOCAL : Local UNIX protocol
// PF_PACKET : Low-level-socket protocol
// PF_IPX : IPX novel protocol
// **** Address Type ****
// AF_INET : IPv4
// AF_INET6 : IPv6
// AF_LOCAL : Local UNIX address
#define SOCKET_TYPE     SOCK_DGRAM
#define PROTO_TYPE      PF_INET
#define ADDR_TYPE       AF_INET


/******************************************
 * Definition of Public Function.
 */ 
CPVD_UDP::CPVD_UDP(AliasType& alias_list)
: IPVDInf(alias_list), Cinet_uds(PROTO_TYPE, SOCKET_TYPE, ADDR_TYPE) {
    try{
        LOGD("Called.");
        set_provider_type(enum_c::ProviderType::E_PVDT_TRANS_UDP);
        assert( update_alias_mapper(alias_list) == true );
        _is_continue_ = false;
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }
}

CPVD_UDP::~CPVD_UDP(void) {
    set_provider_type(enum_c::ProviderType::E_PVDT_NOT_DEFINE);
    _is_continue_ = false;
    bzero(&servaddr, sizeof(servaddr));
}

bool CPVD_UDP::init(std::string id, uint16_t port, const char* ip, ProviderMode mode) {
    /** UDP don't car about mode. Because, UDP has not classification of SERVER/CLIENT. */ 

    set_id(id);

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

    // tag flag
    started = false;
    inited = true;
    return inited;
}

bool CPVD_UDP::start(AppCallerType &app, std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager) {
    
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

bool CPVD_UDP::stop(void) {
    if (sockfd) {
        Close(sockfd);
        sockfd = 0;
    }

    return true;
}

bool CPVD_UDP::accept(void) {
    if(started) {
        std::string client_addr;

        // sockfd is not client-socket-fd so, We will insert value-Zero(0) to parameter-2.
        _is_continue_ = true;
        if (thread_this_migrate(client_addr, std::bind(&CPVD_UDP::run_receiver, this, _1, _2), &_is_continue_) == false) {
            LOGERR("%d: %s", errno, strerror(errno));
            return false;
        }
        return true;
    }

    return false;
}

int CPVD_UDP::make_connection(std::string alias) {
    bool is_new = false;
    try{
        if( mAddr.is_there(alias) ) {
            auto address = mAddr.get(alias);
            assert( mAddr.insert(alias, address, get_provider_type(), is_new, true) == true );
            if( is_new == true ) {
                // trig connected call-back to app.
                hHprotocol->handle_connection(alias, true);
            }
            return true;
        }
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        throw ;
    }
    return false;
}

void CPVD_UDP::disconnection(std::string alias) {
    bool had_connected = false;

    try{
        if( mAddr.is_there(alias) ) {
            mAddr.reset_connect_flag(alias, had_connected);

            if ( had_connected ) {
                // trig connected call-back to app.
                hHprotocol->handle_connection(alias, false);
            }
        }
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        throw ;
    }
}

CPVD_UDP::MessageType CPVD_UDP::read_msg(int u_sockfd, bool &is_new) {
    size_t msg_size = read_bufsize;
    
    u_sockfd = this->sockfd;
    assert(u_sockfd != 0 && read_buf != NULL && read_bufsize > 0);

    MessageType msg = std::make_shared<CRawMessage>();

    try {
        auto cliaddr = std::make_shared<struct sockaddr_in>();
        struct sockaddr_in * p_cliaddr = cliaddr.get();
        socklen_t clilen = sizeof( *p_cliaddr );
        std::string client_addr_str;
        std::lock_guard<std::mutex> guard(mtx_read);

        while(msg_size == read_bufsize) {
            bzero(p_cliaddr, clilen);

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
            std::string client_id = make_client_id(ADDR_TYPE, *p_cliaddr);
            assert( client_id.empty() == false );
            
            // Assumption : We will receive continuous-messages from identical-one-client.
            assert( client_addr_str.empty() == true || client_addr_str.compare(client_id) == 0);
            client_addr_str = client_id;

            if( msg_size > 0 ) {
                assert(msg->append_msg(read_buf, msg_size) == true);
            }
        }

        assert( mAddr.insert(client_addr_str, cliaddr, get_provider_type(), is_new, true) == true );
        msg->set_source(cliaddr, client_addr_str.c_str());
    }
    catch(const std::exception &e) {
        LOGERR("%d: %s", errno, strerror(errno));
        msg->destroy();
        throw ;
    }
    return msg;
}

bool CPVD_UDP::write_msg(std::string alias, MessageType msg) {
    assert( msg.get() != NULL );
    using RawDataType = CRawMessage::MsgDataType;

    int u_sockfd = this->sockfd;
    size_t msg_size = msg->get_msg_size();
    RawDataType* buffer = (RawDataType*)msg->get_msg_read_only();
    struct sockaddr_in* p_cliaddr = NULL;
    socklen_t clilen = sizeof( struct sockaddr_in );

    // soket check. if socket == 0, then we will copy default socket-number.
    assert(u_sockfd != 0 && buffer != NULL && msg_size > 0);

    // alias is prepered. but, if alias is null, then we will use alias registed by msg.
    if ( alias.empty() == true ) {
        alias = msg->get_source_alias();
        if (alias.empty() == true) {
            throw std::invalid_argument("alias is NULL & msg doesn't have alias. Please check it.");
        }
    }

    // if need trig about connection-call-back, then try it.
    if( make_connection(alias) == true ) {
        p_cliaddr = mAddr.get(alias).get();
    }
    else {
        std::string err_str = "alias("+alias+") is not pre-naming in provider-pkg.";
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
int CPVD_UDP::enable_keepalive(int sock) {
    return 0;
}

void CPVD_UDP::update_alias_mapper(AliasType& alias_list, 
                                     std::string &res_alias_name) {
    using AddressType = struct sockaddr_in;
    LOGD("Called.");
    bool is_new = false;
    uint16_t port_num = 0;
    AliasType::iterator itor;
    std::string alias_name;
    std::shared_ptr<cf_alias::CAliasTrans> alias;
    std::shared_ptr<struct sockaddr_in> destaddr;
    assert(alias_list.size() == 1);

    try {
        for ( itor = alias_list.begin(); itor != alias_list.end(); itor++ ) {
            alias.reset();
            destaddr.reset();
            is_new = false;
            
            alias = std::static_pointer_cast<cf_alias::CAliasTrans>(*itor);
            assert(alias.get() != NULL);
            destaddr = std::make_shared<struct sockaddr_in>();
            assert( alias->pvd_type == get_provider_type());

            // make sockaddr_in variables.
            port_num = alias->port_num;
            set_ip_port(*destaddr.get(), alias->ip.c_str(), port_num, ProviderMode::E_PVDM_SERVER);

            // append pair(alias & address) to mapper.
            mAddr.insert(alias->alias, destaddr, alias->pvd_type, is_new);
            res_alias_name = mAddr.get( std::forward<const AddressType>(*destaddr.get()) );
        }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        throw ;
    }
}

bool CPVD_UDP::update_alias_mapper(AliasType& alias_list) {
    LOGD("Called.");
    bool res = true;
    bool is_new = false;
    uint16_t port_num = 0;
    AliasType::iterator itor;
    std::string alias_name;
    std::shared_ptr<cf_alias::CAliasTrans> alias;
    std::shared_ptr<struct sockaddr_in> destaddr;
    
    try {
        for ( itor = alias_list.begin(); itor != alias_list.end(); itor++ ) {
            alias.reset();
            destaddr.reset();
            is_new = false;

            alias = std::static_pointer_cast<cf_alias::CAliasTrans>(*itor);
            assert(alias.get() != NULL);
            destaddr = std::make_shared<struct sockaddr_in>();
            assert( alias->pvd_type == get_provider_type());

            // make sockaddr_in variables.
            port_num = alias->port_num;
            set_ip_port(*destaddr.get(), alias->ip.c_str(), port_num, ProviderMode::E_PVDM_SERVER);

            // append pair(alias & address) to mapper.
            mAddr.insert(alias->alias, destaddr, alias->pvd_type, is_new);
        }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        res = false;
        throw ;
    }

    return res;
}

void CPVD_UDP::run_receiver(std::string alias, bool *is_continue) {
    LOGI("Called with alias(%s)", alias.c_str());

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
                    hHprotocol->handle_connection(msg_raw->get_source_alias(), true);
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

/******************************************
 * Definition of Private Function.
 */ 
std::string CPVD_UDP::make_client_id(const int addr_type, const struct sockaddr_in& cliaddr) {
    std::string client_id;

    // Only support TCP/UDP M2M communication.
    assert( get_provider_type() == enum_c::ProviderType::E_PVDT_TRANS_TCP || 
            get_provider_type() == enum_c::ProviderType::E_PVDT_TRANS_UDP || 
            get_provider_type() == enum_c::ProviderType::E_PVDT_TRANS_UDS );

    try{
        if ( mAddr.is_there(cliaddr) == true ) {
            // Already exist address, then get alias in map.
            client_id = mAddr.get(cliaddr);
        }
        else {
            // If unknown destination, then make new alias.
            auto ipport = get_ip_port(cliaddr);

            if (strcmp(ipport->ip, "0.0.0.0") != 0) {
                client_id = ipport->ip;
                client_id += ':' + std::to_string(ipport->port);
            }
        }
    }
    catch(const std::exception &e){
        LOGERR("%s", e.what());
    }

    LOGD("%s is connected.", client_id.c_str());
    return client_id;
}
