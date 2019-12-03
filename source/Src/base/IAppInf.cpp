#include <cassert>
#include <memory>

#include <IAppInf.h>
#include <server/CServerUDP.h>
#include <server/CServerTCP.h>

/*****
 * Static Function.
 */ 
std::shared_ptr<ICommunicator> create_server(std::string app_id, 
                                             std::string server_id, 
                                             enum_c::ServerType server_type, 
                                             unsigned short port, 
                                             const char* ip) {
    return std::make_shared<ICommunicator>(app_id, server_id, server_type, port, ip);
}

/*****
 * Public Member Function.
 */ 
ICommunicator::ICommunicator(std::string app_id, 
              std::string server_id, 
              enum_c::ServerType server_type, 
              unsigned short port, 
              const char* ip) : runner_continue(false) {

    this->app_id = app_id;
    this->server_id = server_id;
    this->server_type = server_type;
    this->port = port;
    this->ip = ip;

    this->m_sendto = NULL;
    this->m_send_payload = NULL;
}

ICommunicator::~ICommunicator(void) {
    if( runner.joinable() == true ) {
        this->runner_continue = false;
        this->runner.join();
    }

    this->server_type = enum_c::ServerType::E_SERVER_NOT_DEF;
    this->app_id = "destroyed";
    this->server_id = "destroyed";
    this->port = 0;
    this->ip.clear();

    this->m_sendto = NULL;
    this->m_send_payload = NULL;
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

std::string ICommunicator::get_app_id(void) { 
    return app_id; 
}

bool ICommunicator::send(std::string client_id, std::shared_ptr<CPayload>&& payload) {
    if (m_send_payload == NULL) {
        return false;
    }

    m_send_payload(client_id, std::forward<std::shared_ptr<CPayload>>(payload));
    return true;
}

bool ICommunicator::send(std::string client_id, const void* msg, size_t msg_size) {
    if (m_sendto == NULL) {
        return false;
    }

    this->m_sendto(client_id, msg, msg_size);
    return true;
}

/*****
 * Protected Member Function.
 */ 
void ICommunicator::set_send_payload_fp(SendPayloadType &&fp) {
    assert(fp!=NULL);
    this->m_send_payload = fp;
}

void ICommunicator::set_sendto_fp(SendToType &&fp) {
    assert(fp!=NULL);
    this->m_sendto = fp;
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
    app_caller->set_sendto_of_app = std::bind(&ICommunicator::set_sendto_fp, this, std::placeholders::_1);
    app_caller->set_send_payload_of_app = std::bind(&ICommunicator::set_send_payload_fp, this, std::placeholders::_1);
    app_caller->get_cb_handlers = std::bind(&ICommunicator::get_cb_handlers, this);
    app_caller->get_app_id = std::bind(&ICommunicator::get_app_id, this);

    switch(this->server_type)
    {
    case enum_c::ServerType::E_SERVER_TCP:
        {
            auto server = std::make_shared<CServerTCP>();
            server->init(server_id, port, ip.c_str());
            server->start();
            while(server->accept(app_caller) && is_running_continue());
        }
        break;
    case enum_c::ServerType::E_SERVER_UDP:
        {
            auto server = std::make_shared<CServerUDP>();
            server->init(server_id, port, ip.c_str());
            server->start();
            while(server->accept(app_caller) && is_running_continue());
        }
        break;
    default:
        break;
    }
    
    return NULL;
}