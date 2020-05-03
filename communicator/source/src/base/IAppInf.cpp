/***
 * IAppInf.cpp
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
#include <IAppInf.h>
#include <provider/CPVD_TCP.h>
#include <provider/CPVD_UDP.h>

/*****
 * Static Function.
 */ 
std::shared_ptr<ICommunicator> create_communicator(std::string app_id, 
                                                   std::string provider_id, 
                                                   enum_c::ProviderType provider_type, 
                                                   unsigned short port, 
                                                   const char* ip,
                                                   enum_c::ProviderMode mode,
                                                   const char* protocol_desp_path,
                                                   const char* alias_desp_path) {
    try {
            std::shared_ptr<cf_proto::CConfigProtocols> proto_config = std::make_shared<cf_proto::CConfigProtocols>(protocol_desp_path);
            std::shared_ptr<cf_alias::CConfigAliases> alias_config = std::make_shared<cf_alias::CConfigAliases>(alias_desp_path);
            return std::make_shared<ICommunicator>(app_id, provider_id, provider_type, proto_config, alias_config, port, ip, mode);
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        throw ;
    }
}

/*****
 * Public Member Function.
 */ 
ICommunicator::ICommunicator(std::string app_id, 
              std::string provider_id, 
              enum_c::ProviderType provider_type, 
              std::shared_ptr<cf_proto::CConfigProtocols> &proto_config,
              std::shared_ptr<cf_alias::CConfigAliases> &alias_config,
              unsigned short port, 
              const char* ip,
              enum_c::ProviderMode mode) {
    clear();

    try {
        this->app_id = app_id;
        this->provider_id = provider_id;
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
        throw ;
    }
}

ICommunicator::~ICommunicator(void) {
    quit();     // thread quit
    clear();    // variables clear
}

std::string ICommunicator::get_app_id(void) { 
    return app_id; 
}

std::string ICommunicator::get_version(void) {
    return STRING_OF_COMMON_API_VERSION;
}

void ICommunicator::init(void) {
    LOGD("Called.");
    this->runner_continue = true;
    this->runner = std::thread(&ICommunicator::run, this);
}

void ICommunicator::quit(void) {
    if( runner.joinable() == true ) {
        this->runner_continue = false;
        force_exit_thread(runner);
    }
}

void ICommunicator::register_initialization_handler(InitialCB_Type &&handler) {
    assert( handler != NULL );
    cb_handlers.cb_initialization_handle = handler;
}

void ICommunicator::register_connection_handler(ConnectionCB_Type &&handler) {
    assert( handler != NULL );
    cb_handlers.cb_connection_handle = handler;
}

void ICommunicator::register_message_handler(MessagePayloadCB_Type &&handler) {
    assert( handler != NULL );
    cb_handlers.cb_message_payload_handle = handler;
}

void ICommunicator::register_unintended_quit_handler(QuitCB_Type &&handler) {
    assert( handler != NULL );
    cb_handlers.cb_quit_handle = handler;
}

std::shared_ptr<payload::CPayload> ICommunicator::create_payload(void) {
    assert( proto_config->is_ready() == true );
    return proto_config->create_protocols_chain();
}

bool ICommunicator::send(std::string alias, std::shared_ptr<payload::CPayload>&& payload) {
    if (m_send_payload == NULL) {
        return false;
    }

    m_send_payload(alias, std::forward<std::shared_ptr<payload::CPayload>>(payload));
    return true;
}

bool ICommunicator::send(std::string alias, std::shared_ptr<payload::CPayload>& payload) {
    if (m_send_payload == NULL) {
        return false;
    }

    m_send_payload(alias, std::move(payload));
    return true;
}

bool ICommunicator::send(std::string alias, const void* msg, size_t msg_size) {
    try {
        auto payload = create_payload();
        assert( payload->set_payload(msg, msg_size) == true );
        return send(alias, payload);
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw ;
    }

    return false;
}

bool ICommunicator::connect_try(std::string &peer_ip, uint16_t peer_port, std::string &new_alias) {
    bool ret = true;
    assert( peer_ip.empty() != true );
    assert( peer_port > 0 );
    assert( new_alias.empty() == false );

    try {
        if( h_pvd.get() == NULL ) {
            throw std::runtime_error("Provider instance is NULL, Please check it.");
        }
        
        assert( h_pvd->register_new_alias(peer_ip.c_str(), peer_port, new_alias) == true );
        if( h_pvd->make_connection(new_alias) <= 0 ) {
            ret = false;
        }
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw ;
    }

    return ret;
}

bool ICommunicator::connect_try(std::string && alias) {
    bool ret = true;
    assert( alias.empty() == false );

    try{
        if( h_pvd.get() == NULL ) {
            throw std::runtime_error("Provider instance is NULL, Please check it.");
        }

        if( h_pvd->make_connection( alias ) <= 0 ) {
            ret = false;
        }
    }
    catch( const std::exception &e) {
        LOGERR("%s", e.what());
        throw ;
    }

    return ret;
}

void ICommunicator::disconnect(std::string & alias) {
    assert( alias.empty() == false );

    try{
        if( h_pvd.get() == NULL ) {
            LOGW("Provider instance is NULL, Please check it.");
            return ;
        }

        h_pvd->disconnection(alias);
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw ;
    }
}

void ICommunicator::disconnect(std::string && alias) {
    assert( alias.empty() == false );

    try{
        if( h_pvd.get() == NULL ) {
            LOGW("Provider instance is NULL, Please check it.");
            return ;
        }

        h_pvd->disconnection(alias);
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw ;
    }
}

/*****
 * Protected Member Function.
 */ 
void ICommunicator::set_send_payload_fp(SendPayloadType &&fp) {
    assert(fp!=NULL);
    this->m_send_payload = fp;
}

CReceiver& ICommunicator::get_cb_handlers(void) {
    return cb_handlers;
}


/*****
 * Private Member Function.
 */ 
static void cb_force_exit_thread(void* app_id) {
    assert(app_id != NULL);
    LOGW("%s Thread is force-exited.", (const char*)app_id);
}

void ICommunicator::clear(void) {
    this->runner_continue = false;
    this->provider_type = enum_c::ProviderType::E_PVDT_NOT_DEFINE;
    this->app_id = "destroyed";
    this->provider_id = "destroyed";
    this->port = 0;
    this->ip.clear();
    this->mode=enum_c::ProviderMode::E_PVDM_NONE;
    this->proto_config.reset();
    this->alias_config.reset();

    this->m_send_payload = NULL;
    this->h_pvd.reset();
}

void ICommunicator::validation_check(void) {
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

bool ICommunicator::is_running_continue(void) {
    return runner_continue;
}

int ICommunicator::run(void) {
    LOGD("Called.");
    std::shared_ptr<CAppInternalCaller> app_caller = std::make_shared<CAppInternalCaller>();
    app_caller->set_send_payload_of_app = std::bind(&ICommunicator::set_send_payload_fp, this, std::placeholders::_1);
    app_caller->get_cb_handlers = std::bind(&ICommunicator::get_cb_handlers, this);
    app_caller->get_app_id = std::bind(&ICommunicator::get_app_id, this);
    h_pvd.reset();

    // when receive 'cancel' signal, terminate this thread immediatly.
    pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, NULL );
    pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, NULL );
    pthread_cleanup_push( cb_force_exit_thread, (void*)(get_app_id().c_str()) );

    switch(this->provider_type)
    {
    case enum_c::ProviderType::E_PVDT_TRANS_TCP:
        {
            h_pvd = std::make_shared<CPVD_TCP>( alias_config->get_aliases(alias_config->TCP) );
            const char* ip_str = ip.empty() == true ? NULL : ip.c_str();
            h_pvd->init(provider_id, port, ip_str, mode);
            h_pvd->start(app_caller, proto_config);
            while(h_pvd->accept() && is_running_continue());  // Block
        }
        break;
    case enum_c::ProviderType::E_PVDT_TRANS_UDP:
        {
            h_pvd = std::make_shared<CPVD_UDP>( alias_config->get_aliases(alias_config->UDP) );
            const char* ip_str = ip.empty() == true ? NULL : ip.c_str();
            h_pvd->init(provider_id, port, ip_str, mode);
            h_pvd->start(app_caller, proto_config);
            while(h_pvd->accept() && is_running_continue());  // Block
        }
        break;
    default:
        break;
    }

    h_pvd.reset();
    pthread_cleanup_pop(0);     // Macro End of 'pthread_cleanup_push'
    LOGD("Soft-Exit of thread.");
    return NULL;
}

void ICommunicator::force_exit_thread(std::thread &h_thread) {
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
