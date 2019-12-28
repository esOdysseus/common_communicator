
#include <unistd.h>
#include <string.h> 
#include <arpa/inet.h>

#include <logger.h>
#include <IServerInf.h>
#include <IHProtocolInf.h>
#include <protocol/CPBigEndian.h>
#include <protocol/CPLittleEndian.h>
#include <server/CHProtoBaseLan.h>

template class IServerInf<CHProtoBaseLan>;

static const unsigned short client_bufsize = 20;

template <typename PROTOCOL_H> 
class IServerInf<PROTOCOL_H>::CLooper {
public:
    template <typename _Callable>
    explicit CLooper(_Callable&& __f, HProtocolType& instance);

    ~CLooper(void);

    bool joinable(void) { return (h_thread ? h_thread->joinable(): false); }

    void join(void) { if(h_thread) h_thread->join(); }

private: 
    HProtocolType h_protocol;

    ThreadType h_thread;
};

/**************************************************
 * Definition for Member-Function of CLooper Class.
 */
template <typename PROTOCOL_H> 
template<typename _Callable>
IServerInf<PROTOCOL_H>::CLooper::CLooper(_Callable&& __f, HProtocolType& instance) {
    this->h_protocol = move(instance);
    this->h_thread = std::make_shared<std::thread>(std::forward<_Callable>(__f), this->h_protocol);
}

template <typename PROTOCOL_H> 
IServerInf<PROTOCOL_H>::CLooper::~CLooper(void) {
    if( this->h_thread != nullptr && this->h_thread->joinable() ) {
        this->h_thread->join();
    }
    this->h_thread.reset();
    this->h_protocol.reset();
}


/**************************************************
 * Definition for Member-Function of IServerInf Class.
 */
template <typename PROTOCOL_H> 
IServerInf<PROTOCOL_H>::IServerInf(void) {
    id = "";
    clear();
}

template <typename PROTOCOL_H> 
IServerInf<PROTOCOL_H>::~IServerInf(void) {
    stop();
}

template <typename PROTOCOL_H> 
bool IServerInf<PROTOCOL_H>::stop(void) {
    int res = 0;

    // close socket.
    started = false;
    usleep(10000);       // wait 10 msec

    if (sockfd) {
        res = close(sockfd);
        if(res < 0) {
            LOGW("socket closing is failed.");
        }
        sockfd = 0;
    }

    // mLooperPool remove
    for( auto itor = mLooperPool.begin(); itor != mLooperPool.end(); itor++ ) {
        itor->second.reset();
    }

    // clear variables.
    clear();
    return true;
}

template <typename PROTOCOL_H> 
void IServerInf<PROTOCOL_H>::clear(void) {
    // clear All-member-variables
    inited = false;
    started = false;
    sockfd = 0;
    server_type = enum_c::ServerType::E_SERVER_NOT_DEF;
    bzero(&servaddr, sizeof(servaddr));
    listeningPort = 0;
    mLooperPool.clear();
}

template <typename PROTOCOL_H> 
bool IServerInf<PROTOCOL_H>::thread_create(std::string& client_id, int socketfd, 
                                           AppCallerType& app, Json_DataType &json_manager) {
    try {
        HProtocolType h_protocol = std::make_shared<PROTOCOL_H>(client_id, socketfd, 
                                                      SharedThisType::shared_from_this(), 
                                                      app, json_manager);
        
        if ( mLooperPool.find(client_id) == mLooperPool.end() ) {
            mLooperPool.emplace(client_id, std::make_shared<CLooper>(&IHProtocolInf::run, h_protocol));
        }
        else {
            LOGW("Already, thread was created by %s", client_id.c_str());
        }
        return true;
    } catch (const std::exception &e) {
        LOGERR("%d: %s", errno, strerror(errno));
    }
    return false;
}

template <typename PROTOCOL_H> 
bool IServerInf<PROTOCOL_H>::thread_this_migrate(std::string& client_id, int socketfd, 
                                                 AppCallerType& app, Json_DataType &json_manager) {
    try {
        if (client_id.empty()) {
            client_id = "ALL_CLIENT";
        }
        
        HProtocolType h_protocol = std::make_shared<PROTOCOL_H>(client_id, socketfd, 
                                                      SharedThisType::shared_from_this(), 
                                                      app, json_manager);
        h_protocol->run();
        return true;
    } catch (const std::exception &e) {
        LOGERR("%d: %s", errno, strerror(errno));
    }
    return false;
}

template <typename PROTOCOL_H> 
std::string IServerInf<PROTOCOL_H>::make_client_id(const int addr_type, const struct sockaddr_in& cliaddr) {
    std::string client_id;

    // Only support TCP/UDP M2M communication.
    assert( get_server_type() == enum_c::ServerType::E_SERVER_TCP || 
            get_server_type() == enum_c::ServerType::E_SERVER_UDP );

    try{
        char client_addr[client_bufsize] = {0,};

        inet_ntop(addr_type, &cliaddr.sin_addr.s_addr, client_addr, sizeof(client_addr));
        LOGD("Server : %s:%d  client connected.", client_addr, cliaddr.sin_port);

        if (strcmp(client_addr, "0.0.0.0") != 0) {
            client_id = client_addr;
            client_id += ':' + std::to_string(cliaddr.sin_port);
        }
    }
    catch(const std::exception &e){
        LOGERR("%s", e.what());
    }

    return client_id;
}

