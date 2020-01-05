#include <cassert>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <cassert>
#include <memory>

#include <logger.h>
#include <CRawMessage.h>
#include <server/CServerUDP.h>

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


CServerUDP::CServerUDP(void)
: IServerInf() {
    set_server_type(enum_c::ServerType::E_SERVER_UDP);
    clear_addr();
}

CServerUDP::~CServerUDP(void) {
    set_server_type(enum_c::ServerType::E_SERVER_NOT_DEF);
    clear_addr();
}

bool CServerUDP::init(std::string id, unsigned int port, const char* ip) {
    set_id(id);

    if (inited == true) {
        LOGERR("Already Init() is called. Please check it.");
        return inited;
    }

    // Input data checking.
    if(port == 0 || port >= 65535) {
        LOGERR("No port defined to listen to");
        return false;
    }

    // make UDP-Socket
    sockfd = socket(PROTO_TYPE, SOCKET_TYPE, 0);
    if (sockfd < 0) {
        LOGERR("%d: %s", errno, strerror(errno));
        return false;
    }

    // activate Keep-Alive.
    // enable_keepalive(sockfd);

    // make serveraddr struct
    listeningPort = port;
    servaddr.sin_family = ADDR_TYPE;
    if ( ip == NULL ) {
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }else {
        servaddr.sin_addr.s_addr = inet_addr(ip);
    }
    servaddr.sin_port = htons(listeningPort);
    //Set all bits of the padding field to 0 
    memset(servaddr.sin_zero, '\0', sizeof(servaddr.sin_zero));

    // tag flag
    started = false;
    inited = true;
    return inited;
}

bool CServerUDP::start(void) {
    
    if (inited != true) {
        LOGERR("We need to init ServerTCP. Please check it.");
        return false;
    }

    // bind socket & server-address.
    if(bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) {
        LOGERR("%d: %s", errno, strerror(errno));
        return false;
    }

    started = true;
    return started;
}

bool CServerUDP::accept(AppCallerType &app, std::shared_ptr<CConfigProtocols> &proto_manager) {
    if(started) {
        std::string client_addr;
        // sockfd is not client-socket-fd so, We will insert value-Zero(0) to parameter-2.
        if (thread_this_migrate(client_addr, 0, app, proto_manager) == false) {
            LOGERR("%d: %s", errno, strerror(errno));
            return false;
        }
        return true;
    }

    return false;
}

CServerUDP::MessageType CServerUDP::read_msg(int u_sockfd, bool &is_new) {
    size_t msg_size = read_bufsize;
    
    if (u_sockfd == 0) {
        u_sockfd = this->sockfd;
    }
    assert(u_sockfd != 0 && read_buf != NULL && read_bufsize > 0);

    MessageType msg = std::make_shared<CRawMessage>(u_sockfd);

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

            if( msg_size > 0 ){
                assert(msg->append_msg(read_buf, msg_size) == true);
            }
        }

        assert( insert_addr(client_addr_str, *p_cliaddr, is_new) == true );
        msg->set_source(cliaddr, client_addr_str.c_str());
    }
    catch(const std::exception &e) {
        LOGERR("%d: %s", errno, strerror(errno));
        msg->destroy();
    }
    return msg;
}

bool CServerUDP::write_msg(std::string client_id, MessageType msg) {
    assert( msg.get() != NULL );
    assert( isthere_addr(client_id) == true );
    using RawDataType = CRawMessage::MsgDataType;

    int u_sockfd = msg->get_socket_fd();
    size_t msg_size = msg->get_msg_size();
    RawDataType* buffer = (RawDataType*)msg->get_msg_read_only();
    const struct sockaddr_in* p_cliaddr = msg->get_source_addr_read_only(get_server_type());
    socklen_t clilen = sizeof( *p_cliaddr );

    // If client_alias_name == null, then find address corresfond client-id.
    if (msg->get_source_alias().empty() == true) {
        p_cliaddr = get_addr(client_id).get();
    }

    if (u_sockfd == 0) {
        u_sockfd = this->sockfd;
    }
    assert(u_sockfd != 0 && buffer != NULL && msg_size > 0);

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
    }
    catch(const std::exception &e) {
        LOGERR("%d: %s", errno, strerror(errno));
        return false;
    }
    return true;
}

int CServerUDP::enable_keepalive(int sock) {
    return 0;
}

bool CServerUDP::isthere_addr(std::string alias) {
    return ( m_alias_addr.find(alias) != m_alias_addr.end() ? true : false );
}

CServerUDP::AddressType CServerUDP::get_addr(std::string alias) {
    AddressType address;
    
    // Find Address correspond with Client-ID.
    if ( m_alias_addr.find(alias) != m_alias_addr.end() ) {
        address = m_alias_addr[alias];
    }
    return address;
}

bool CServerUDP::insert_addr(std::string alias, const struct sockaddr_in &address, bool & is_new) {
    bool result = false;

    try {
        if( alias.empty() == true ) {
            return result;
        }

        is_new = false;
        if ( m_alias_addr.find(alias) == m_alias_addr.end() ) {
            m_alias_addr[alias] = std::make_shared<struct sockaddr_in>();
            struct sockaddr_in* p_addr = m_alias_addr[alias].get();
            memcpy(p_addr, &address, sizeof(address));
            is_new = true;
        }
        result = true;
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }
    return result;
}

void CServerUDP::remove_addr(std::string alias) {
    AddressMapType::iterator itor = m_alias_addr.find(alias);
    if (itor != m_alias_addr.end()) {
        m_alias_addr.erase(itor);
    }
}

void CServerUDP::clear_addr(void) {
    m_alias_addr.clear();
}