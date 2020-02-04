#include <cassert>
#include <logger.h>
#include <Enum_common.h>
#include <IAppInf.h>
#include <server/CHProtoBaseLan.h>

using namespace std::placeholders;

CHProtoBaseLan::CHProtoBaseLan(std::string client_addr, int socket_handler, 
                   ServerType &&server, AppCallerType& app, std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager)
: IHProtocolInf(client_addr, socket_handler, app, proto_manager) {
    this->s_server = std::forward<ServerType>(server);
}

CHProtoBaseLan::~CHProtoBaseLan(void) {
    destroy();
    s_server.reset();
}

bool CHProtoBaseLan::set_app_call_back(void) {
    AppCallerType& app = get_app_instance();

    app->set_send_payload_of_app(bind(&CHProtoBaseLan::write_payload, this, _1, _2));
    app->get_cb_handlers().cb_initialization_handle(s_server->get_provider_type(), true);
    return true;
}

/************************
 * This API support that setting Specific-property of 3th-party Protocol.
 */ 
bool CHProtoBaseLan::write_payload(std::string alias, std::shared_ptr<payload::CPayload>&& payload) {
    LOGD("It's called.");
    assert( get_running_flag() == true );

    try {
        ProtocolType pro_payload = std::dynamic_pointer_cast<IProtocolInf>(payload);
        SegmentsType segments = encapsulation(pro_payload, s_server->get_provider_type());

        // message write.
        for(SegmentsType::iterator itor = segments.begin(); itor != segments.end(); itor++) {
            assert(s_server->write_msg(alias, *itor) == true);
        }

        if ( payload->get_op_flag(payload::E_PAYLOAD_FLAG::E_KEEP_PAYLOAD_AFTER_TX) == false ) {
            destroy_proto_chain(pro_payload);
        }
        else {
            LOGW("E_KEEP_PAYLOAD_AFTER_TX == true");
        }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

void CHProtoBaseLan::run(void) {
    set_running_flag(true);
    AppCallerType& app = get_app_instance();
    
    // application call-back setting
    assert( set_app_call_back() == true );

    LOGD("Thread_ID   : %s", get_thread_id().c_str());
    LOGD("App_ID      : %s", app->get_app_id().c_str());
    LOGD("Server_ID   : %s", s_server->get_id().c_str());
    LOGD("Server_Type : %d", s_server->get_provider_type());

    // trig initial-call-back to application.
    if( s_server->get_provider_type() == enum_c::ProviderType::E_PVDT_TRANS_TCP ) {
        app->get_cb_handlers().cb_connection_handle( get_client_id(), true );
    }

    try {
        while(get_running_flag()) {
            bool is_new = false;
            RawMsgType msg_raw;

            // check received message 
            msg_raw = s_server->read_msg(get_sockfd(), is_new);     // get raw message. (Blocking)
            if( is_new == true ) {
                app->get_cb_handlers().cb_connection_handle( msg_raw->get_source_alias(), true );
            }

            // message parsing with regard to PROTOCOL.
            ProtocolType p_msg = decapsulation(msg_raw);
            if(p_msg->is_empty() == false) {
                app->get_cb_handlers().cb_message_payload_handle(msg_raw->get_source_alias(),
                                                                 p_msg);  // trig app-function.
            }
            destroy_proto_chain(p_msg);
        }
        app->get_cb_handlers().cb_initialization_handle(s_server->get_provider_type(), false);
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        app->get_cb_handlers().cb_quit_handle(e);
        destroy();
    }
}

bool CHProtoBaseLan::destroy(void) {
    set_running_flag(false);
    return true;
}
