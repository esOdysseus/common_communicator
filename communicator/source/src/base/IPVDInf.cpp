/***
 * IPVDInf.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */

#include <unistd.h>
#include <string.h> 
#include <arpa/inet.h>

#include <logger.h>
#include <IPVDInf.h>
#include <IHProtocolInf.h>
#include <provider/CHProtoBaseLan.h>

template bool IPVDInf::create_hprotocol<CHProtoBaseLan>(AppCallerType& app, std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager);

constexpr const unsigned int IPVDInf::BUFSIZE_READ;

class IPVDInf::CLooper {
public:
    template <typename _Callable>
    explicit CLooper(_Callable&& __f, std::shared_ptr<cf_alias::IAliasPVD> peer_alias);

    ~CLooper(void);

    bool joinable(void) { return (h_thread ? h_thread->joinable(): false); }

    void join(void) { if(h_thread) h_thread->join(); }

    void force_join(void);

    bool is_thread_continue(void) { return is_continue; };

private: 
    ThreadType h_thread;

    bool is_continue;
};

/**************************************************
 * Definition for Member-Function of CLooper Class.
 */
template<typename _Callable>
IPVDInf::CLooper::CLooper(_Callable&& __f, std::shared_ptr<cf_alias::IAliasPVD> peer_alias) {
    this->is_continue = true;
    this->h_thread = std::make_shared<std::thread>(std::forward<_Callable>(__f), peer_alias, &is_continue);
    assert(this->h_thread.get() != NULL);
}

IPVDInf::CLooper::~CLooper(void) {
    force_join();
}

void IPVDInf::CLooper::force_join(void) {
    if ( this->h_thread.get() != NULL ) {
        this->is_continue = false;

        if( this->h_thread->joinable() ) {
            this->h_thread->join();
        }
        this->h_thread.reset();
    }
}


/**************************************************
 * Definition for Public Member-Function of IPVDInf Class.
 */

IPVDInf::IPVDInf(std::shared_ptr<cf_alias::IAliasPVD>& pvd_alias, 
                 std::shared_ptr<cf_alias::CConfigAliases>& alias_manager,
                 unsigned int bufsize_read) {
    try {
        LOGD("Called.");
        read_buf = NULL;
        clear();

        if ( bufsize_read <= 512U ) {
            throw std::invalid_argument("bufsize_read have to be number more than 512 bytes.");
        }
        if ( pvd_alias.get() == NULL ) {
            throw std::invalid_argument("pvd_alias is invalid data.");
        }

        // Make read-buffer.
        read_bufsize = bufsize_read;
        read_buf = new char[read_bufsize];
        if( read_buf == NULL ) {
            throw std::runtime_error("Can not allocate memory to read_buf.");
        }

        _m_pvd_alias_ = pvd_alias; 
        _m_config_alias_ = alias_manager;

        // if myself is un-registed pvd_alias, then register it to config_alias.
        regist_unknown_pvd_alias( _m_pvd_alias_ );
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        quit();
        throw e;
    }
}

IPVDInf::~IPVDInf(void) {
    quit();
}

bool IPVDInf::register_new_alias(const char* peer_ip, uint16_t peer_port, 
                                 std::string& app_path, std::string &pvd_id) {
    using PvdType = cf_alias::CAliasTrans;
    enum_c::ProviderType pvd_type = get_provider_type();
    assert(peer_ip != NULL);
    assert(peer_port > 0);
    assert(pvd_id.empty() == false);

    try {
        if ( pvd_type != enum_c::ProviderType::E_PVDT_TRANS_TCP && 
            pvd_type != enum_c::ProviderType::E_PVDT_TRANS_UDP && 
            pvd_type != enum_c::ProviderType::E_PVDT_TRANS_UDS_TCP &&
            pvd_type != enum_c::ProviderType::E_PVDT_TRANS_UDS_UDP ) {
            throw std::domain_error("IP/Port is only allowed within TCP/UDP/UDS.");
        }

        // convert String-type ip/port to Abstracted IAliasPVD-type.
        auto new_alias = std::make_shared<PvdType>(app_path.data(), pvd_id.data(), PvdType::convert(pvd_type).data());
        new_alias->set_ip( peer_ip );
        new_alias->set_port( peer_port );
        new_alias->set_mask( 24 );

        // append new alias to internal data-structure of Provider.
        update_alias_mapper(new_alias);
        return true;
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return false;
}

bool IPVDInf::quit(void) {
    // close socket.
    started = false;
    usleep(10000);       // wait 10 msec

    assert( stop() == true );

    // mLooperPool remove
    {
        std::unique_lock<std::mutex> guard(mtx_looperpool);

        for( auto itor = mLooperPool.begin(); itor != mLooperPool.end();) {
            std::string alias = itor->first;
            itor++;
            thread_destroy(alias);
        }
    }

    // clear variables.
    clear();
    return true;
}

enum_c::ProviderType IPVDInf::get_provider_type(void) { 
    if( _m_pvd_alias_.get() != NULL ) {
        return _m_pvd_alias_->type();
    }

    return enum_c::ProviderType::E_PVDT_NOT_DEFINE;
}

std::shared_ptr<cf_alias::IAliasPVD> IPVDInf::get_pvd_alias( void ) { 
    return _m_pvd_alias_; 
}


/*******************************************************
 * Private Function Definition of IPVDInf Class.
 */
void IPVDInf::clear(void) {
    // clear All-member-variables
    inited = false;
    started = false;
    sockfd = 0;
    listeningPort = 0;
    {
        std::unique_lock<std::mutex> guard(mtx_looperpool);
        mLooperPool.clear();
    }
    {
        std::unique_lock<std::mutex> guard(mtx_loopergarbage);
        mLooperGarbage.clear();
    }
    hHprotocol.reset();
    _m_pvd_alias_.reset();
    _m_config_alias_.reset();

    // Delete read-buffer.
    if ( read_buf != NULL ) {
        delete [] read_buf;
        read_buf = NULL;
    }
    read_bufsize = 0;
}

template <typename PROTOCOL_H> 
bool IPVDInf::create_hprotocol(AppCallerType& app, 
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

bool IPVDInf::thread_create(std::shared_ptr<cf_alias::IAliasPVD>& peer_alias, FPreceiverType &&func) {
    try {
        assert(peer_alias.get() != NULL);
        std::unique_lock<std::mutex> guard(mtx_looperpool);

        if ( mLooperPool.find(peer_alias->path()) == mLooperPool.end() ) {
            mLooperPool.emplace(peer_alias->path(), std::make_shared<CLooper>( func, peer_alias));
        }
        else {
            LOGW("Already, thread was created by %s", peer_alias->path().data());
        }
        return true;
    } catch (const std::exception &e) {
        LOGERR("%d: %s", errno, strerror(errno));
    }
    return false;
}

bool IPVDInf::thread_this_migrate(std::shared_ptr<cf_alias::IAliasPVD>& peer_alias, FPreceiverType &&func, bool *is_continue) {
    assert(is_continue != NULL);

    try {
        func( peer_alias, is_continue );

        return true;
    } catch (const std::exception &e) {
        LOGERR("%s", e.what());
        LOGERR("%d: %s", errno, strerror(errno));
    }
    return false;
}

bool IPVDInf::thread_destroy(std::string peer_full_path) {
    try{
        if ( mLooperPool.find(peer_full_path) != mLooperPool.end() ) {
            auto itor = mLooperPool.find(peer_full_path);
            itor->second->force_join();
            itor->second.reset();
            mLooperPool.erase(itor);

            assert( mLooperPool.find(peer_full_path) == mLooperPool.end() );
        }
        return true;
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
    }

    return false;
}

bool IPVDInf::zombi_thread_migrate(std::shared_ptr<cf_alias::IAliasPVD> peer_alias) {
    try{
        assert(peer_alias.get() != NULL);
        std::string peer_full_path = peer_alias->path();
        std::unique_lock<std::mutex> guard(mtx_looperpool);

        if ( mLooperPool.find(peer_full_path) != mLooperPool.end() ) {
            auto itor = mLooperPool.find(peer_full_path);
            {
                std::unique_lock<std::mutex> guard(mtx_loopergarbage);
                mLooperGarbage[itor->first] = itor->second;
            }
            mLooperPool.erase(itor);

            assert( mLooperPool.find(peer_full_path) == mLooperPool.end() );
        }
        return true;
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
    }

    return false;
}

bool IPVDInf::regist_connected_peer(std::shared_ptr<cf_alias::IAliasPVD> peer_alias) {
    try {
        // register IAliasPVD & peer_full_path to config_alias.
        return _m_config_alias_->regist_connected_peer( peer_alias, _m_pvd_alias_ );
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
    }

    return false;
}

void IPVDInf::unregist_connected_peer( std::string peer_full_path ) {
    try {
        // unregister relationship of IAliasPVD & peer_full_path in config_alias.
        auto my_pvd_full_path = _m_pvd_alias_->path();
        _m_config_alias_->unregist_connected_peer( peer_full_path, my_pvd_full_path );
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
    }
}

void IPVDInf::regist_unknown_pvd_alias(std::shared_ptr<cf_alias::IAliasPVD> pvd_alias) {
    try {
        // register IAliasPVD to config_alias if not exist. 
        _m_config_alias_->regist_provider( pvd_alias );     // update [app, pvd]
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}