/***
 * IServerInf.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <random>

#include <unistd.h>
#include <string.h> 
#include <arpa/inet.h>

#include <logger.h>
#include <IServerInf.h>
#include <IHProtocolInf.h>
#include <server/CHProtoBaseLan.h>

template bool IServerInf::create_hprotocol<CHProtoBaseLan>(AppCallerType& app, std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager);

class IServerInf::CLooper {
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
template<typename _Callable>
IServerInf::CLooper::CLooper(_Callable&& __f, std::string alias) {
    this->is_continue = true;
    this->h_thread = std::make_shared<std::thread>(std::forward<_Callable>(__f), alias, &is_continue);
    assert(this->h_thread.get() != NULL);
}

IServerInf::CLooper::~CLooper(void) {
    force_join();
}

void IServerInf::CLooper::force_join(void) {
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

IServerInf::IServerInf(AliasType& alias_list) {
    try {
        LOGD("Called.");
        id = "";
        clear();
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        stop();
        throw e;
    }
}

IServerInf::~IServerInf(void) {
    stop();
}

bool IServerInf::register_new_alias(const char* peer_ip, uint16_t peer_port, 
                                    std::string &wanted_name) {
    using AliasType = cf_alias::CAliasTrans;
    std::string str_pvd_type;
    std::list<std::shared_ptr<cf_alias::IAlias>> alias_list;
    assert(peer_ip != NULL);
    assert(peer_port > 0);
    assert(wanted_name.empty() == false);

    try {
        if ( get_provider_type() != enum_c::ProviderType::E_PVDT_TRANS_TCP && 
            get_provider_type() != enum_c::ProviderType::E_PVDT_TRANS_UDP && 
            get_provider_type() != enum_c::ProviderType::E_PVDT_TRANS_UDS ) {
            throw std::domain_error("IP/Port is only allowed within TCP/UDP/UDS.");
        }

        // convert String-type ip/port to Abstracted IAlias-type.
        str_pvd_type = AliasType::get_pvd_type(get_provider_type());
        auto alias = std::make_shared<AliasType>(wanted_name.c_str(), str_pvd_type.c_str());
        alias->ip = peer_ip;
        alias->port_num = peer_port;
        alias->mask = 24;
        alias_list.push_back(alias);

        // append new alias to internal data-structure of Provider.
        update_alias_mapper(alias_list, wanted_name);
        return true;
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw ;
    }

    return false;
}

bool IServerInf::stop(void) {
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

int IServerInf::gen_random_portnum(void) {
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

void IServerInf::clear(void) {
    // clear All-member-variables
    inited = false;
    started = false;
    sockfd = 0;
    provider_type = enum_c::ProviderType::E_PVDT_NOT_DEFINE;
    listeningPort = 0;
    mLooperPool.clear();
    hHprotocol.reset();
}

template <typename PROTOCOL_H> 
bool IServerInf::create_hprotocol(AppCallerType& app, 
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

bool IServerInf::thread_create(std::string& client_id, FPreceiverType &&func) {
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

bool IServerInf::thread_this_migrate(std::string& client_id, FPreceiverType &&func, bool *is_continue) {
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

bool IServerInf::thread_destroy(std::string client_id) {
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



