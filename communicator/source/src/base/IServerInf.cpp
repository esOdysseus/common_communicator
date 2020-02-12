#include <random>

#include <unistd.h>
#include <string.h> 
#include <arpa/inet.h>

#include <logger.h>
#include <IServerInf.h>
#include <IHProtocolInf.h>
#include <server/CHProtoBaseLan.h>

template class IServerInf<CHProtoBaseLan>;

static const unsigned short client_bufsize = 20;

template <typename PROTOCOL_H> 
class IServerInf<PROTOCOL_H>::CLooper {
public:
    template <typename _Callable>
    explicit CLooper(_Callable&& __f, std::string alias);

    ~CLooper(void);

    bool joinable(void) { return (h_thread ? h_thread->joinable(): false); }

    void join(void) { if(h_thread) h_thread->join(); }

    void force_join(void);

private: 
    ThreadType h_thread;

    bool is_continue;
};

/**************************************************
 * Definition for Member-Function of CLooper Class.
 */
template <typename PROTOCOL_H> 
template<typename _Callable>
IServerInf<PROTOCOL_H>::CLooper::CLooper(_Callable&& __f, std::string alias) {
    this->is_continue = true;
    this->h_thread = std::make_shared<std::thread>(std::forward<_Callable>(__f), alias, &is_continue);
    assert(this->h_thread.get() != NULL);
}

template <typename PROTOCOL_H> 
IServerInf<PROTOCOL_H>::CLooper::~CLooper(void) {
    force_join();
}

template <typename PROTOCOL_H> 
void IServerInf<PROTOCOL_H>::CLooper::force_join(void) {
    if ( this->h_thread.get() != NULL && this->is_continue == true) {
        this->is_continue = false;

        if( this->h_thread->joinable() ) {
            this->h_thread->join();
        }
        this->h_thread.reset();
    }
}


/**************************************************
 * Definition for Member-Function of IServerInf Class.
 */
template <typename PROTOCOL_H> 
IServerInf<PROTOCOL_H>::IServerInf(AliasType& alias_list) {
    try {
        id = "";
        clear();
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        stop();
        throw e;
    }
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
int IServerInf<PROTOCOL_H>::gen_random_portnum(void) {
    int port = -1;
    int port_min = 10000;
    int port_max = 60000;
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen( rd() ); 
    std::uniform_int_distribution<> dist(port_min, port_max); 
    
    LOGD("Random Port-Min : %d", dist.min());
    LOGD("Random Port-Max : %d", dist.max());

    port = dist( gen );
    assert(port_min <= port && port <= port_max);
    LOGI("Generated Port-Number=%d", port);

    return port;
}

template <typename PROTOCOL_H> 
void IServerInf<PROTOCOL_H>::clear(void) {
    // clear All-member-variables
    inited = false;
    started = false;
    sockfd = 0;
    provider_type = enum_c::ProviderType::E_PVDT_NOT_DEFINE;
    bzero(&servaddr, sizeof(servaddr));
    listeningPort = 0;
    mLooperPool.clear();
    hHprotocol.reset();
}

template <typename PROTOCOL_H> 
bool IServerInf<PROTOCOL_H>::create_hprotocol(AppCallerType& app, 
                                              std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager) {
    try{
        hHprotocol.reset();
        hHprotocol = std::make_shared<PROTOCOL_H>(SharedThisType::shared_from_this(), 
                                                  app, proto_manager);
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

template <typename PROTOCOL_H> 
bool IServerInf<PROTOCOL_H>::thread_create(std::string& client_id, FPreceiverType &&func) {
    try {
        if ( mLooperPool.find(client_id) == mLooperPool.end() ) {
            mLooperPool.emplace(client_id, std::make_shared<CLooper>( func, client_id));
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
bool IServerInf<PROTOCOL_H>::thread_this_migrate(std::string& client_id, FPreceiverType &&func, bool *is_continue) {
    assert(is_continue != NULL);

    try {
        if (client_id.empty()) {
            client_id = "ALL_CLIENT";
        }

        func(client_id, is_continue);
        return true;
    } catch (const std::exception &e) {
        LOGERR("%d: %s", errno, strerror(errno));
    }
    return false;
}

template <typename PROTOCOL_H> 
bool IServerInf<PROTOCOL_H>::thread_destroy(std::string client_id) {
    try{
        if ( mLooperPool.find(client_id) == mLooperPool.end() ) {
            auto itor = mLooperPool.find(client_id);
            itor->second->force_join();
            itor->second.reset();
            mLooperPool.erase(itor);
        }
        return true;
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
    }

    return false;
}

template <typename PROTOCOL_H> 
std::string IServerInf<PROTOCOL_H>::make_client_id(const int addr_type, const struct sockaddr_in& cliaddr) {
    std::string client_id;
    char client_addr[client_bufsize] = {0,};
    int port_num = -1;

    // Only support TCP/UDP M2M communication.
    assert( get_provider_type() == enum_c::ProviderType::E_PVDT_TRANS_TCP || 
            get_provider_type() == enum_c::ProviderType::E_PVDT_TRANS_UDP );

    try{
        if ( mAddr.is_there(cliaddr) == true ) {
            // Already exist address, then get alias in map.
            client_id = mAddr.get(cliaddr);
        }
        else {
            // If unknown destination, then make new alias.
            port_num = ntohs(cliaddr.sin_port);
            inet_ntop(addr_type, &cliaddr.sin_addr.s_addr, client_addr, sizeof(client_addr));

            if (strcmp(client_addr, "0.0.0.0") != 0) {
                client_id = client_addr;
                client_id += ':' + std::to_string(port_num);
            }
        }
    }
    catch(const std::exception &e){
        LOGERR("%s", e.what());
    }

    LOGD("%s is connected.", client_id.c_str());
    return client_id;
}

