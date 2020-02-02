#include <cassert>
#include <memory>

#include <config.h>
#include <logger.h>
#include <CConfigProtocols.h>
#include <CConfigAliases.h>
#include <IAppInf.h>
#include <server/CServerUDP.h>
#include <server/CServerTCP.h>

/*****
 * Static Function.
 */ 
std::shared_ptr<ICommunicator> create_communicator(std::string app_id, 
                                                   std::string provider_id, 
                                                   enum_c::ProviderType provider_type, 
                                                   unsigned short port, 
                                                   const char* ip,
                                                   const char* protocol_desp_path,
                                                   const char* alias_desp_path) {
    std::shared_ptr<ICommunicator> ret;
    try {
            std::shared_ptr<cf_proto::CConfigProtocols> proto_config = std::make_shared<cf_proto::CConfigProtocols>(protocol_desp_path);
            std::shared_ptr<cf_alias::CConfigAliases> alias_config = std::make_shared<cf_alias::CConfigAliases>(alias_desp_path);
            ret = std::make_shared<ICommunicator>(app_id, provider_id, provider_type, proto_config, port, ip);
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
    }

    return ret;
}

/*****
 * Public Member Function.
 */ 
ICommunicator::ICommunicator(std::string app_id, 
              std::string provider_id, 
              enum_c::ProviderType provider_type, 
              std::shared_ptr<cf_proto::CConfigProtocols> &proto_config,
              unsigned short port, 
              const char* ip) 
: runner_continue(false) {

    this->app_id = app_id;
    this->provider_id = provider_id;
    this->provider_type = provider_type;
    this->port = port;
    if ( ip != NULL ) {
        this->ip = ip;
    }
    this->proto_config = proto_config;

    this->m_send_payload = NULL;
}

ICommunicator::~ICommunicator(void) {
    if( runner.joinable() == true ) {
        this->runner_continue = false;
        this->runner.join();
    }

    this->provider_type = enum_c::ProviderType::E_PVDT_NOT_DEFINE;
    this->app_id = "destroyed";
    this->provider_id = "destroyed";
    this->port = 0;
    this->ip.clear();
    this->proto_config.reset();

    this->m_send_payload = NULL;
}

std::string ICommunicator::get_app_id(void) { 
    return app_id; 
}

std::string ICommunicator::get_version(void) {
    return STRING_OF_COMMON_API_VERSION;
}

void ICommunicator::init(void) {
    this->runner_continue = true;
    this->runner = std::thread(&ICommunicator::run, this);
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

void ICommunicator::register_quit_handler(QuitCB_Type &&handler) {
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
        throw e;
    }

    return false;
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
bool ICommunicator::is_running_continue(void) {
    return runner_continue;
}

int ICommunicator::run(void) {

    std::shared_ptr<CAppInternalCaller> app_caller = std::make_shared<CAppInternalCaller>();
    app_caller->set_send_payload_of_app = std::bind(&ICommunicator::set_send_payload_fp, this, std::placeholders::_1);
    app_caller->get_cb_handlers = std::bind(&ICommunicator::get_cb_handlers, this);
    app_caller->get_app_id = std::bind(&ICommunicator::get_app_id, this);

    switch(this->provider_type)
    {
    case enum_c::ProviderType::E_PVDT_TRANS_TCP:
        {
            auto pvd = std::make_shared<CServerTCP>();
            const char* ip_str = ip.empty() == true ? NULL : ip.c_str();
            pvd->init(provider_id, port, ip_str);
            pvd->start();
            while(pvd->accept(app_caller, proto_config) && is_running_continue());
        }
        break;
    case enum_c::ProviderType::E_PVDT_TRANS_UDP:
        {
            auto pvd = std::make_shared<CServerUDP>();
            const char* ip_str = ip.empty() == true ? NULL : ip.c_str();
            pvd->init(provider_id, port, ip_str);
            pvd->start();
            while(pvd->accept(app_caller, proto_config) && is_running_continue());
        }
        break;
    default:
        break;
    }
    
    return NULL;
}
