#include <cassert>
#include <logger.h>
#include <Enum_common.h>
#include <IAppInf.h>
#include <IProtocolInf.h>
#include <server/CHProtoBaseLan.h>
#include <protocol/CPBigEndian.h>
#include <protocol/CPLittleEndian.h>

template class CHProtoBaseLan<CPBigEndian>;
template class CHProtoBaseLan<CPLittleEndian>;
template class CHProtoBaseLan<CNoneProtocol>;

using namespace std;
using namespace std::placeholders;

template <typename PROTOCOL>
CHProtoBaseLan<PROTOCOL>::CHProtoBaseLan(std::string client_addr, int socket_handler, 
                   ServerType &&server, AppCallerType& app)
: IHProtocolInf(client_addr, socket_handler, std::forward<ServerType>(server), app) {
    this->s_server = std::forward<ServerType>(server);
}

template <typename PROTOCOL>
CHProtoBaseLan<PROTOCOL>::~CHProtoBaseLan(void) {
    destroy();
    s_server.reset();
}

template <typename PROTOCOL>
bool CHProtoBaseLan<PROTOCOL>::set_app_call_back(void) {
    AppCallerType& app = get_app_instance();

    app->set_sendto_of_app(bind(&CHProtoBaseLan::write, this, _1, _2, _3));
    app->set_send_payload_of_app(bind(&CHProtoBaseLan::write_payload, this, _1, _2));
    app->get_cb_handlers().cb_initialization_handle(s_server->get_server_type(), true);
    return true;
}

/************************
 * This API not support that setting Specific-property of 3th-party Protocol.
 */ 
template <typename PROTOCOL>
bool CHProtoBaseLan<PROTOCOL>::write(std::string client_id, const void* msg, size_t msg_size) {
    LOGD("It's called.");
    try {
        SegmentsType segments = encapsulation<PROTOCOL>(msg, msg_size, s_server->get_server_type());

        // message write.
        for(SegmentsType::iterator itor = segments.begin(); itor != segments.end(); itor++) {
            assert(s_server->write_msg(client_id, *itor) == true);
        }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

/************************
 * This API support that setting Specific-property of 3th-party Protocol.
 */ 
template <typename PROTOCOL>
bool CHProtoBaseLan<PROTOCOL>::write_payload(std::string client_id, std::shared_ptr<CPayload>&& payload) {
    LOGD("It's called.");
    try {
        std::shared_ptr<PROTOCOL> pro_payload = std::dynamic_pointer_cast<PROTOCOL>(payload);
        SegmentsType segments = encapsulation<PROTOCOL>(pro_payload, s_server->get_server_type());

        // message write.
        for(SegmentsType::iterator itor = segments.begin(); itor != segments.end(); itor++) {
            assert(s_server->write_msg(client_id, *itor) == true);
        }
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        return false;
    }
    return true;
}

template <typename PROTOCOL>
void CHProtoBaseLan<PROTOCOL>::run(void) {
    set_running_flag(true);
    AppCallerType& app = get_app_instance();
    
    // application call-back setting
    assert( set_app_call_back() == true );

    LOGD("Thread_ID   : %s", get_thread_id().c_str());
    LOGD("App_ID      : %s", app->get_app_id().c_str());
    LOGD("Server_ID   : %s", s_server->get_id().c_str());
    LOGD("Server_Type : %d", s_server->get_server_type());

    // trig initial-call-back to application.
    if( s_server->get_server_type() == enum_c::ServerType::E_SERVER_TCP ) {
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

            // if(msg_raw->get_msg_size() > 0) {
            //     LOGD("Received MSG : %s", (const char*)(msg_raw->get_msg_read_only()));
            //     LOGD("Client Info  : %s", msg_raw->get_source_alias().c_str());
            // }

            // message parsing with regard to PROTOCOL.
            ProtocolType p_msg = decapsulation<PROTOCOL>(msg_raw);
            if(p_msg->is_there_data() == true) {
                app->get_cb_handlers().cb_message_payload_handle(msg_raw->get_source_alias(),
                                                                 p_msg);  // trig app-function.
            }
        }
        app->get_cb_handlers().cb_initialization_handle(s_server->get_server_type(), false);
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        app->get_cb_handlers().cb_quit_handle(e);
        destroy();
    }
}

template <typename PROTOCOL>
bool CHProtoBaseLan<PROTOCOL>::destroy(void) {
    set_running_flag(false);
    return true;
}
