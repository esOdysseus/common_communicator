#include <iostream>
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
#define SOCKET_TYPE     SOCK_STREAM
#define PROTO_TYPE      PF_INET
#define ADDR_TYPE       AF_INET

typedef enum E_ERROR {
    E_TCP_NO_ERROR = 0,
    E_TCP_UNKNOWN_ALIAS = 1,
    E_TCP_HAVE_NOT_ALIAS = 2,
    E_TCP_NOT_SUPPORT_CLOUD_CONNECTION = 3,
    E_TCP_SESSION_CLOSED = 4,
    E_TCP_CREATE_THREAD_FAILED = 5,
    E_TCP_ACCEPT_FAILED = 6,
    E_TCP_CONNECT_FAILED = 7,
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


CServerTCP::CServerTCP(AliasType& alias_list)
: IServerInf(alias_list) {
    try{
        set_provider_type(enum_c::ProviderType::E_PVDT_TRANS_TCP);
        assert( update_alias_mapper(alias_list) == true );
        m_alias2socket.clear();
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }
}

CServerTCP::~CServerTCP(void) {
    CAliasAddr<int>::AddrIterator itor;
    set_provider_type(enum_c::ProviderType::E_PVDT_NOT_DEFINE);
    
    for( itor = m_alias2socket.begin(); itor != m_alias2socket.end(); itor++ ) {
        int u_sockfd = *(m_alias2socket.get(itor->first).get());
        thread_destroy(itor->first);
        usleep(10000);      // for wait thread delete-complete.
        close(u_sockfd);
    }
    m_alias2socket.clear();
}

bool CServerTCP::init(std::string id, unsigned int port, const char* ip) {
    set_id(id);

    if (inited == true) {
        LOGERR("Already Init() is called. Please check it.");
        return inited;
    }

    // Input data checking.
    if(port < 0 || port >= 65535) {
        LOGERR("No port defined to listen to");
        return false;
    }

    // make socket
    assert( (sockfd = make_socket(1)) > 0 );

    // make serveraddr struct
    listeningPort = port;
    if (listeningPort == 0) {
        listeningPort = gen_random_portnum();
    }

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

bool CServerTCP::start(AppCallerType &app, std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager) {
    
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

    // Create instance of Protocol-Handler.
    assert( create_hprotocol(app, proto_manager) == true );
    assert( hHprotocol->handle_initialization(get_provider_type(), true) == true );

    started = true;
    return started;
}

bool CServerTCP::accept(void) {
    if(started) {
        bool is_new = false;
        std::string client_id;
        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(cliaddr);
        std::shared_ptr<int> address = std::make_shared<int>();

        try{
            // start accept-blocking.
            int newsockfd = ::accept(sockfd, (struct sockaddr*) &cliaddr, &clilen); // Blocking Function.
            if(newsockfd < 0) {
                LOGERR("%d: %s", errno, strerror(errno));
                throw CException(E_ERROR::E_TCP_ACCEPT_FAILED);
            }

            // Get client-ID
            client_id = make_client_id(ADDR_TYPE, cliaddr);
            *(address.get()) = newsockfd;
            assert( m_alias2socket.insert(client_id, address, get_provider_type(), is_new) == true );

            if ( client_id.empty() == false ) {
                // create thread with PROTOCOL for new-sesseion by new-user.
                if (thread_create(client_id, std::bind(&CServerTCP::run_receiver, this, _1, _2)) == false) {
                    LOGERR("%d: Thread Create failed: %s", errno, strerror(errno));
                    assert( disconnection(client_id) == true);
                    throw CException(E_ERROR::E_TCP_CREATE_THREAD_FAILED);
                }
            }

            return true;
        }
        catch( const std::exception &e ) {
            LOGERR("%s", e.what());
        }
    }

    return false;
}

int CServerTCP::make_connection(std::string alias) {
    bool is_new = false;
    int new_sockfd = -1;
    struct sockaddr_in *destaddr = NULL;
    std::shared_ptr<int> address = std::make_shared<int>();

    try{
        if ( alias.find("http://") != std::string::npos ||
             alias.find("https://") != std::string::npos ) {
            // TODO: try connect to cloud base on TCP.
            // destaddr = cloud_connection(alias);
            throw CException(E_ERROR::E_TCP_NOT_SUPPORT_CLOUD_CONNECTION);
        }
        else if( mAddr.is_there(alias) == true ) {
            destaddr = (struct sockaddr_in*)mAddr.get(alias).get();
        }
        else {
            throw CException(E_ERROR::E_TCP_UNKNOWN_ALIAS);
        }

        assert( (new_sockfd = make_socket(1)) > 0 );
        if( connect(new_sockfd, (struct sockaddr *)destaddr, sizeof(*destaddr)) >= 0 ) {
            LOGD("Connection to DEST.(%s) is success.", alias.c_str());

            // register new_sockfd to m_alias2socket.
            *(address.get()) = new_sockfd;
            assert( m_alias2socket.insert(alias, address, get_provider_type(), is_new) == true );
            assert( is_new == true );
            
            // create thread with PROTOCOL for new-sesseion by new-user.
            if (thread_create(alias, std::bind(&CServerTCP::run_receiver, this, _1, _2)) == false) {
                LOGERR("%d: Thread Create failed: %s", errno, strerror(errno));
                assert( disconnection(alias) == true);
                throw CException(E_ERROR::E_TCP_CREATE_THREAD_FAILED);
            }
        }
        else {
            LOGW("Connection to DEST.(%s) is failed.", alias.c_str());
            close(new_sockfd);
            throw CException(E_ERROR::E_TCP_CONNECT_FAILED);
        }
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        new_sockfd = -1;
        throw e;
    }

    return new_sockfd;
}

bool CServerTCP::disconnection(std::string alias) {
    int u_sockfd = -1;

    if ( m_alias2socket.is_there(alias) == false) {
        LOGW("alias is not exist in m_alias2socket mapper.");
        return false;
    }

    try {
        u_sockfd = *(m_alias2socket.get(alias).get());

        // TODO need auto-lock
        close( u_sockfd );
        m_alias2socket.remove(alias);
        return true;
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return false;
}

CServerTCP::MessageType CServerTCP::read_msg(int u_sockfd, bool &is_new) {
    ssize_t msg_size = read_bufsize;
    assert(u_sockfd > 0 && read_buf != NULL && read_bufsize > 0);
    MessageType msg = std::make_shared<CRawMessage>();
    is_new = false;

    try {
        while(msg_size == read_bufsize) {
            // return value description
            // 0 : End of Field.
            // -1 : Error. (session is Closed by client)
            // > 0 : The number of received message.
            msg_size = read(u_sockfd, read_buf, read_bufsize);    // Blocking Function.
            if(msg_size < 0) {
                LOGW("Session is closed by Client. MSG-Receiving is failed.");
                throw CException(E_ERROR::E_TCP_SESSION_CLOSED);
            }
            assert( msg_size >= 0 && msg_size <= read_bufsize);
            if( msg_size > 0 ){
                assert(msg->append_msg(read_buf, msg_size) == true);
            }
        }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        msg->destroy();
        throw e;
    }
    return msg;
}

bool CServerTCP::write_msg(std::string alias, MessageType msg) {
    assert( msg.get() != NULL );
    using RawDataType = CRawMessage::MsgDataType;

    bool res = true;
    int u_sockfd = -1;
    ssize_t msg_size = msg->get_msg_size();
    RawDataType* buffer = (RawDataType*)msg->get_msg_read_only();

    // alias is prepered. but, if alias is null, then we will use alias registed by msg.
    u_sockfd = get_socket(alias, msg);
    assert(u_sockfd > 0 && buffer != NULL && msg_size > 0);

    try {
        ssize_t written_size = 0;
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
        LOGERR("%s", e.what());
        res = false;
    }
    return res;
}

/************************************
 * Definition of Protected Function.
 */
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

bool CServerTCP::update_alias_mapper(AliasType& alias_list) {
    bool res = true;

    try {
        AliasType::iterator itor;

        for ( itor = alias_list.begin(); itor != alias_list.end(); itor++ ) {
            bool is_new = false;
            std::shared_ptr<cf_alias::CAliasTrans> alias = std::static_pointer_cast<cf_alias::CAliasTrans>(*itor);
            std::shared_ptr<struct sockaddr_in> destaddr = std::make_shared<struct sockaddr_in>();
            assert( alias->pvd_type == get_provider_type());

            // make sockaddr_in variables.
            destaddr->sin_family = ADDR_TYPE;
            destaddr->sin_addr.s_addr = inet_addr(alias->ip.c_str());
            destaddr->sin_port = htons(alias->port_num);
            // set all bits of the padding field to 0
            memset(destaddr->sin_zero, '\0', sizeof(destaddr->sin_zero));

            // append pair(alias & address) to mapper.
            mAddr.insert(alias->alias, destaddr, alias->pvd_type, is_new);
        }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        res = false;
        throw e;
    }

    return res;
}

void CServerTCP::run_receiver(std::string alias, bool *is_continue) {
    LOGI("Called with alias(%s)", alias.c_str());
    int socket_fd = -1;
    bool is_new = false;
    MessageType msg_raw;
    std::shared_ptr<int> socket = std::make_shared<int>();
    
    try {
        assert(is_continue != NULL && *is_continue == true);
        assert( m_alias2socket.is_there(alias) == true );
        assert( (socket_fd = *(m_alias2socket.get(alias).get())) > 0 );
        *(socket.get()) = socket_fd;

        // trig connected call-back to app.
        assert( hHprotocol->handle_connection(alias, true) == true );

        // Start receiver
        LOGD("Start MSG-receiver.");
        while(*is_continue) {
            is_new = false;
            msg_raw.reset();

            // check received message 
            msg_raw = read_msg(socket_fd, is_new);     // get raw message. (Blocking)
            msg_raw->set_source(socket, alias.c_str());
            // trig handling of protocol & Call-back to app
            assert( hHprotocol->handle_protocol_chain(msg_raw) == true );
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
int CServerTCP::make_socket(int opt_flag) {
    int new_sockfd = -1;

    // make TCP-Socket
    if( (new_sockfd = socket(PROTO_TYPE, SOCKET_TYPE, 0)) < 0 ) {
        LOGERR("%d: %s", errno, strerror(errno));
        return false;
    }
    if(setsockopt(new_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt_flag, sizeof(opt_flag)) == -1) {
        LOGERR("%d: %s", errno, strerror(errno));
        return false;
    }

    // activate Keep-Alive.
    enable_keepalive(new_sockfd);

    return new_sockfd;
}

int CServerTCP::get_socket(std::string alias, MessageType &msg) {
    int u_sockfd = -1;

    try {
        // alias is prepered. but, if alias is null, then we will use alias registed by msg.
        if ( alias.empty() == false ) {
            if ( m_alias2socket.is_there(alias) == true ) {
                u_sockfd = *(m_alias2socket.get(alias).get());
            }
            else {
                assert( (u_sockfd=make_connection(alias)) > 0 );
            }
        }
        else {  // alias is NULL.
            if (msg->get_source_alias().empty() == false) {
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
