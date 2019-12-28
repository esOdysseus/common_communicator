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

#include <logger.h>
#include <CRawMessage.h>
#include <server/CServerTCP.h>

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
#define SOCKET_TYPE     SOCK_STREAM
#define PROTO_TYPE      PF_INET
#define ADDR_TYPE       AF_INET


CServerTCP::CServerTCP(void)
: IServerInf() {
    set_server_type(enum_c::ServerType::E_SERVER_TCP);
}

CServerTCP::~CServerTCP(void) {
    set_server_type(enum_c::ServerType::E_SERVER_NOT_DEF);
}

bool CServerTCP::init(std::string id, unsigned int port, const char* ip) {
    int yes = 1;

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

    // make TCP-Socket
    if( (sockfd = socket(PROTO_TYPE, SOCKET_TYPE, 0)) < 0 ) {
        LOGERR("%d: %s", errno, strerror(errno));
        return false;
    }
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        LOGERR("%d: %s", errno, strerror(errno));
        return false;
    }

    // activate Keep-Alive.
    enable_keepalive(sockfd);

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

bool CServerTCP::start(void) {
    
    if (inited != true) {
        LOGERR("We need to init ServerTCP. Please check it.");
        return false;
    }

    // bind socket & server-address.
    if(bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) {
        LOGERR("%d: %s", errno, strerror(errno));
        return false;
    }

    // start listen.
    if(listen(sockfd, 5) < 0)
    {
        LOGERR("%d: Server : Can't listening connect: %s", errno, strerror(errno));
        return false;
    }

    started = true;
    return started;
}

bool CServerTCP::accept(AppCallerType &app, Json_DataType &json_manager) {
    if(started) {
        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(cliaddr);

        // start accept-blocking.
        int newsockfd = ::accept(sockfd, (struct sockaddr*) &cliaddr, &clilen); // Blocking Function.
        if(newsockfd < 0) {
            LOGERR("%d: %s", errno, strerror(errno));
            return false;
        }

        // Get client-ID
        std::string client_id = make_client_id(ADDR_TYPE, cliaddr);
        assert( insert_client(newsockfd, client_id) == true );

        if ( client_id.empty() == false ) {
            // create thread with PROTOCOL for new-sesseion by new-user.
            if (thread_create(client_id, newsockfd, app, json_manager) == false) {
                LOGERR("%d: Thread Create failed: %s", errno, strerror(errno));
                return false;
            }
        }

        return true;
    }

    return false;
}

CServerTCP::MessageType CServerTCP::read_msg(int u_sockfd, bool &is_new) {
    size_t msg_size = read_bufsize;
    
    if (u_sockfd == 0) {
        u_sockfd = this->sockfd;
    }
    assert(u_sockfd != 0 && read_buf != NULL && read_bufsize > 0);
    assert(isthere_client_id(u_sockfd) == true);

    std::shared_ptr<int> socket = std::make_shared<int>(u_sockfd);
    MessageType msg = std::make_shared<CRawMessage>(u_sockfd);

    try {
        while(msg_size == read_bufsize) {
            // return value description
            // 0 : End of Field.
            // -1 : Error.
            // > 0 : The number of received message.
            msg_size = read(u_sockfd, read_buf, read_bufsize);    // Blocking Function.
            assert( msg_size >= 0 && msg_size <= read_bufsize);
            if( msg_size > 0 ){
                assert(msg->append_msg(read_buf, msg_size) == true);
            }
        }
        msg->set_source(socket, get_client_id(u_sockfd).c_str());
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        msg->destroy();
    }
    return msg;
}

bool CServerTCP::write_msg(std::string client_id, MessageType msg) {
    assert( msg.get() != NULL );
    using RawDataType = CRawMessage::MsgDataType;

    int u_sockfd = msg->get_socket_fd();
    size_t msg_size = msg->get_msg_size();
    RawDataType* buffer = (RawDataType*)msg->get_msg_read_only();

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
            // > 0 : The number of written message.
            written_size=write(u_sockfd, (const void*)buffer, msg_size);
            assert(written_size > 0);

            msg_size -= written_size;
            buffer = buffer + written_size;
            written_size = 0;
        }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

int CServerTCP::enable_keepalive(int sock) {
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



bool CServerTCP::isthere_client_id(const int socket_num) {
    return ( m_socket_alias.find(socket_num) != m_socket_alias.end() ? true : false );
}

std::string CServerTCP::get_client_id(const int socket_num) {
    std::string client_id;
    
    // Find Address correspond with Client-ID.
    if ( m_socket_alias.find(socket_num) != m_socket_alias.end() ) {
        client_id = m_socket_alias[socket_num];
    }
    return client_id;
}

bool CServerTCP::insert_client(const int socket_num, std::string alias) {
    bool result = false;

    try {
        if( socket_num == NULL || alias.empty() == true ) {
            return result;
        }

        if ( m_socket_alias.find(socket_num) == m_socket_alias.end() ) {
            m_socket_alias[socket_num] = alias;
        }
        result = true;
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }
    return result;
}

void CServerTCP::remove_client(const int socket_num) {
    SocketMapType::iterator itor = m_socket_alias.find(socket_num);
    if (itor != m_socket_alias.end()) {
        m_socket_alias.erase(itor);
    }
}

void CServerTCP::clear_clients(void) {
    m_socket_alias.clear();
}