/***
 * ICommunicator.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <cassert>
#include <memory>

#include <config.h>
#include <logger.h>
#include <CConfigProtocols.h>
#include <CConfigAliases.h>
#include <ICommunicatorImpl.h>
#include <provider/CPVD_TCP.h>
#include <provider/CPVD_UDP.h>


/*****
 * Public Member Function.
 */ 
// Constructor API for Dynamic-Auto parsed Provider.
ICommunicatorImpl::ICommunicatorImpl(std::string app_id, 
                                     std::string provider_id, 
                                     std::shared_ptr<cf_alias::CConfigAliases> &alias_config,
                                     std::shared_ptr<cf_proto::CConfigProtocols> &proto_config,
                                     enum_c::ProviderMode mode) {
    clear();

    try {
        // Get Provider-Type info.
        auto pvd_alias = alias_config->get_provider( app_id, provider_id );
        if ( pvd_alias.get() == NULL ) {
            std::string err = "Provider is not exist. (" + cf_alias::IAlias::make_full_path(app_id, provider_id ) + ")";
            throw std::out_of_range(err);
        }
        this->provider_type = pvd_alias->type();

        // Set Provider Info.
        switch ( this->provider_type ) {
        case enum_c::ProviderType::E_PVDT_TRANS_TCP:
        case enum_c::ProviderType::E_PVDT_TRANS_UDP:
        case enum_c::ProviderType::E_PVDT_TRANS_UDS_TCP:
        case enum_c::ProviderType::E_PVDT_TRANS_UDS_UDP:
            {
                auto pvd_trans = pvd_alias->get<cf_alias::CAliasTrans>();
                this->port = pvd_trans->get_port();
                this->ip = pvd_trans->get_ip_ref();
            }
            break;
        default :
            std::string err = "Can not Supporte Provider Type. (" + std::to_string(provider_type) + ")";
            throw std::out_of_range(err);
        }

        this->app_id = std::move(app_id);
        this->provider_id = std::move(provider_id);
        this->mode = mode;
        this->proto_config = proto_config;
        this->alias_config = alias_config;

        validation_check();
    }
    catch( const std::out_of_range &e ) {
        LOGERR("%s", e.what());
        clear();
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

ICommunicatorImpl::ICommunicatorImpl(std::string app_id, 
                                     std::string provider_id, 
                                     std::shared_ptr<cf_alias::CConfigAliases> &alias_config,
                                     std::shared_ptr<cf_proto::CConfigProtocols> &proto_config,
                                     enum_c::ProviderType provider_type, 
                                     unsigned short port, 
                                     const char* ip,
                                     enum_c::ProviderMode mode) {
    clear();

    try {
        this->app_id = std::move(app_id);
        this->provider_id = std::move(provider_id);
        this->provider_type = provider_type;
        this->port = port;
        if ( ip != NULL ) {
            this->ip = ip;
        }
        this->mode = mode;
        this->proto_config = proto_config;
        this->alias_config = alias_config;

        validation_check();
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

ICommunicatorImpl::~ICommunicatorImpl(void) {
    quit();     // thread quit
    clear();    // variables clear
}

std::string ICommunicatorImpl::get_app_id(void) { 
    return app_id; 
}

std::string ICommunicatorImpl::get_provider_id(void) { 
    return provider_id; 
}

void ICommunicatorImpl::init(void) {
    LOGD("Called.");
    this->runner_continue = true;
    this->runner = std::thread(&ICommunicatorImpl::run, this);
}

void ICommunicatorImpl::quit(void) {
    if( runner.joinable() == true ) {
        this->runner_continue = false;
        force_exit_thread(runner);
    }
}

void ICommunicatorImpl::register_initialization_handler(InitialCB_Type &&handler) {
    assert( handler != NULL );
    cb_handlers.cb_initialization_handle = handler;
}

void ICommunicatorImpl::register_connection_handler(ConnectionCB_Type &&handler) {
    assert( handler != NULL );
    cb_handlers.cb_connection_handle = handler;
}

void ICommunicatorImpl::register_message_handler(MessagePayloadCB_Type &&handler) {
    assert( handler != NULL );
    cb_handlers.cb_message_payload_handle = handler;
}

void ICommunicatorImpl::register_unintended_quit_handler(QuitCB_Type &&handler) {
    assert( handler != NULL );
    cb_handlers.cb_quit_handle = handler;
}

std::shared_ptr<payload::CPayload> ICommunicatorImpl::create_payload(void) {
    assert( proto_config->is_ready() == true );
    return proto_config->create_protocols_chain();
}

bool ICommunicatorImpl::send(std::string app_path, std::string pvd_path, std::shared_ptr<payload::CPayload>&& payload) {
    if (m_send_payload == NULL) {
        return false;
    }

    m_send_payload(std::move(app_path), std::move(pvd_path), std::forward<std::shared_ptr<payload::CPayload>>(payload));
    return true;
}

bool ICommunicatorImpl::send(std::string app_path, std::string pvd_path, std::shared_ptr<payload::CPayload>& payload) {
    if (m_send_payload == NULL) {
        return false;
    }

    m_send_payload(std::move(app_path), std::move(pvd_path), std::move(payload));
    return true;
}

bool ICommunicatorImpl::connect_try(std::string &peer_ip, uint16_t peer_port, std::string& app_path, std::string &pvd_id) {
    bool ret = true;
    std::string peer_full_path;
    assert( peer_ip.empty() != true );
    assert( peer_port > 0 );

    try {
        peer_full_path = cf_alias::IAlias::make_full_path(app_path, pvd_id);
        if( h_pvd.get() == NULL ) {
            throw std::runtime_error("Provider instance is NULL, Please check it.");
        }
        
        assert( h_pvd->register_new_alias(peer_ip.data(), peer_port, app_path, pvd_id) == true );
        if( h_pvd->make_connection(peer_full_path) <= 0 ) {
            ret = false;
        }
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return ret;
}

bool ICommunicatorImpl::connect_try(std::string && app_path, std::string && pvd_id) {
    bool ret = true;
    assert( app_path.empty() == false );
    assert( pvd_id.empty() == false );

    try{
        if( h_pvd.get() == NULL ) {
            throw std::runtime_error("Provider instance is NULL, Please check it.");
        }

        if( h_pvd->make_connection( cf_alias::IAlias::make_full_path(app_path, pvd_id) ) <= 0 ) {
            ret = false;
        }
    }
    catch( const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }

    return ret;
}

void ICommunicatorImpl::disconnect(std::string & app_path, std::string & pvd_id) {
    assert( app_path.empty() == false );
    assert( pvd_id.empty() == false );

    try{
        if( h_pvd.get() == NULL ) {
            LOGW("Provider instance is NULL, Please check it.");
            return ;
        }

        h_pvd->disconnection(app_path, pvd_id);
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

void ICommunicatorImpl::disconnect(std::string && app_path, std::string && pvd_id) {
    assert( app_path.empty() == false );
    assert( pvd_id.empty() == false );

    try{
        if( h_pvd.get() == NULL ) {
            LOGW("Provider instance is NULL, Please check it.");
            return ;
        }

        h_pvd->disconnection(app_path, pvd_id);
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

/*****
 * Private Member Function.
 */ 
void ICommunicatorImpl::set_send_payload_fp(SendPayloadType &&fp) {
    assert(fp!=NULL);
    this->m_send_payload = fp;
}

CReceiver& ICommunicatorImpl::get_cb_handlers(void) {
    return cb_handlers;
}

static void cb_force_exit_thread(void* app_id) {
    assert(app_id != NULL);
    LOGW("%s Thread is force-exited.", (const char*)app_id);
}

void ICommunicatorImpl::clear(void) {
    this->runner_continue = false;
    this->provider_type = enum_c::ProviderType::E_PVDT_NOT_DEFINE;
    this->app_id.clear();
    this->provider_id.clear();
    this->port = 0;
    this->ip.clear();
    this->mode=enum_c::ProviderMode::E_PVDM_NONE;
    this->proto_config.reset();
    this->alias_config.reset();

    this->m_send_payload = NULL;
    this->h_pvd.reset();
}

void ICommunicatorImpl::validation_check(void) {
    if(this->provider_type == enum_c::ProviderType::E_PVDT_NOT_DEFINE) {
        throw std::invalid_argument("Invalid Input: provider_type is NOT_DEFINE.");
    }
    if(this->mode == enum_c::ProviderMode::E_PVDM_NONE) {
        throw std::invalid_argument("Invalid Input: mode is NONE.");
    }
    if(this->app_id.empty() == true) {
        throw std::invalid_argument("Invalid Input: app_id is empty.");
    }
    if(this->provider_id.empty() == true) {
        throw std::invalid_argument("Invalid Input: provider_id is empty.");
    }
}

bool ICommunicatorImpl::is_running_continue(void) {
    return runner_continue;
}

int ICommunicatorImpl::run(void) {
    LOGD("Called.");
    std::shared_ptr<CAppInternalCaller> app_caller = std::make_shared<CAppInternalCaller>();
    app_caller->set_send_payload_of_app = std::bind(&ICommunicatorImpl::set_send_payload_fp, this, std::placeholders::_1);
    app_caller->get_cb_handlers = std::bind(&ICommunicatorImpl::get_cb_handlers, this);
    app_caller->get_app_id = std::bind(&ICommunicatorImpl::get_app_id, this);
    app_caller->get_provider_id = std::bind(&ICommunicatorImpl::get_provider_id, this);
    h_pvd.reset();

    // when receive 'cancel' signal, terminate this thread immediatly.
    pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, NULL );
    pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, NULL );
    pthread_cleanup_push( cb_force_exit_thread, (void*)(get_app_id().c_str()) );

    try {
        auto pvd_alias = get_provider();

        switch(this->provider_type)
        {
        case enum_c::ProviderType::E_PVDT_TRANS_TCP:
            {
                h_pvd = std::make_shared<CPVD_TCP<>>( pvd_alias, alias_config->get_providers(pvd_alias->TCP) );
                h_pvd->init(port, ip, mode);
                h_pvd->start(app_caller, proto_config);
                while(h_pvd->accept() && is_running_continue());  // Block
            }
            break;
        case enum_c::ProviderType::E_PVDT_TRANS_UDP:
            {
                h_pvd = std::make_shared<CPVD_UDP<>>(pvd_alias, alias_config->get_providers(pvd_alias->UDP) );
                h_pvd->init(port, ip, mode);
                h_pvd->start(app_caller, proto_config);
                while(h_pvd->accept() && is_running_continue());  // Block
            }
            break;
        case enum_c::ProviderType::E_PVDT_TRANS_UDS_TCP:
            {
                h_pvd = std::make_shared<CPVD_TCP<struct sockaddr_un>>(pvd_alias, alias_config->get_providers(pvd_alias->TCP_UDS) );
                h_pvd->init(port, ip, mode);
                h_pvd->start(app_caller, proto_config);
                while(h_pvd->accept() && is_running_continue());  // Block
            }
            break;
        case enum_c::ProviderType::E_PVDT_TRANS_UDS_UDP:
            {
                h_pvd = std::make_shared<CPVD_UDP<struct sockaddr_un>>(pvd_alias, alias_config->get_providers(pvd_alias->UDP_UDS) );
                h_pvd->init(port, ip, mode);
                h_pvd->start(app_caller, proto_config);
                while(h_pvd->accept() && is_running_continue());  // Block
            }
            break;
        default:
            break;
        }
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    h_pvd.reset();
    pthread_cleanup_pop(0);     // Macro End of 'pthread_cleanup_push'
    LOGD("Soft-Exit of thread.");
    return 0;
}

void ICommunicatorImpl::force_exit_thread(std::thread &h_thread) {
    if( h_thread.joinable() == true ) {
        auto n_thread = h_thread.native_handle();

        LOGD("Forcefully terminate Thread to join ...");
        h_thread.detach();           // detach point of thread-handler from object.
        pthread_cancel( n_thread );  // foce exit
        LOGD("Done.(joinable=%d)", h_thread.joinable());

        // wait for calling of 'cb_force_exit_thread' function
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

std::shared_ptr<cf_alias::IAliasPVD> ICommunicatorImpl::get_provider( void ) {
    std::shared_ptr<cf_alias::IAliasPVD> res;
    try {
        res = alias_config->get_provider( app_id, provider_id, provider_type );
        if( res.get() == NULL ) {
            res = alias_config->create_provider( app_id, provider_id, provider_type );
        }
        assert( res.get() != NULL );

        if( provider_type == enum_c::ProviderType::E_PVDT_TRANS_TCP || 
            provider_type == enum_c::ProviderType::E_PVDT_TRANS_UDP ) {
            // if type is transaction-type, then check ip/port validation.
            // if need, then update empty ip/port variables.
            auto pvd_trans = res->convert<cf_alias::CAliasTrans>( res );
            sync_trans_provider( pvd_trans );
        }
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return res;
}

void ICommunicatorImpl::sync_trans_provider( std::shared_ptr<cf_alias::CAliasTrans>& pvd ) {
    try {
        std::string& pvd_ip = pvd->get_ip_ref();
        uint32_t& pvd_port = pvd->get_port_ref();

        if( pvd_ip.empty() == true && ip.empty() == false ) {
            pvd_ip = ip;
            pvd_port = port;
            pvd->set_mask(24);
        }
        else if( pvd_ip.empty() == false && ip.empty() == true ) {
            ip = pvd_ip;
            port = pvd_port;
        }
        else if( pvd_ip.empty() == false && ip.empty() == false ) {
            if( pvd_ip != ip || pvd_port != port ) {
                LOGERR("IP/Port(%s/%u) is not missmatched with IP/Port(%s/%u) of Provider.", ip.data(), port, pvd_ip.data(), pvd_port);
                throw std::invalid_argument( "IP/Port value is mismatched with IP/Port of Provider." );
            }
        }
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}
