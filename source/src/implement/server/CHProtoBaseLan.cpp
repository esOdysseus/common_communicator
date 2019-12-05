#include <cassert>
#include <iostream>
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
    cout << "[Debug] CHProtoBaseLan::write() is called." << endl;
    try {
        SegmentsType segments = encapsulation<PROTOCOL>(msg, msg_size, s_server->get_server_type());

        // message write.
        for(SegmentsType::iterator itor = segments.begin(); itor != segments.end(); itor++) {
            assert(s_server->write_msg(client_id, *itor) == true);
        }
    }
    catch(const std::exception &e) {
        cout << "[Error] CHProtoBaseLan::write() : " << e.what() << endl;
        return false;
    }
    return true;
}

/************************
 * This API support that setting Specific-property of 3th-party Protocol.
 */ 
template <typename PROTOCOL>
bool CHProtoBaseLan<PROTOCOL>::write_payload(std::string client_id, std::shared_ptr<CPayload>&& payload) {
    cout << "[Debug] CHProtoBaseLan::write() is called." << endl;
    try {
        std::shared_ptr<PROTOCOL> pro_payload = std::dynamic_pointer_cast<PROTOCOL>(payload);
        SegmentsType segments = encapsulation<PROTOCOL>(pro_payload, s_server->get_server_type());

        // message write.
        for(SegmentsType::iterator itor = segments.begin(); itor != segments.end(); itor++) {
            assert(s_server->write_msg(client_id, *itor) == true);
        }
    }
    catch(const std::exception &e) {
        cout << "[Error] CHProtoBaseLan::write() : " << e.what() << endl;
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

    cout << "Thread_ID : " << get_thread_id() << endl;
    cout << "App_ID : " << app->get_app_id() << endl;
    cout << "Server_ID : " << s_server->get_id() << endl;
    cout << "Server_Type : " << s_server->get_server_type() << endl;

    // trig initial-call-back to application.
    if( s_server->get_server_type() == enum_c::ServerType::E_SERVER_TCP ) {
        app->get_cb_handlers().cb_connection_handle( get_client_id(), true );
    }

    try {
        while(get_running_flag()) {
            RawMsgType msg_raw;

            // check received message 
            msg_raw = s_server->read_msg(get_sockfd());     // get raw message. (Blocking)
            if(msg_raw->get_msg_size() > 0) {
                cout << "Received MSG : " << (const char*)(msg_raw->get_msg_read_only()) << endl;
                cout << "Client Info : " << msg_raw->get_source_alias() << endl;
            }

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
        cout << "[Error] CHProtoBaseLan::run() : " << e.what() << endl;
        app->get_cb_handlers().cb_quit_handle(e);
        destroy();
    }
}

template <typename PROTOCOL>
bool CHProtoBaseLan<PROTOCOL>::destroy(void) {
    set_running_flag(false);
    return true;
}
