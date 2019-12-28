
#include <cassert>
#include <list>
#include <unistd.h>

#include <logger.h>
#include <IHProtocolInf.h>

/**********************************
 * Public Function definition.
 */
IHProtocolInf::~IHProtocolInf(void) {
    if (h_socket != 0) {
        close(h_socket);
    }
    t_id = "destroyed";
    h_socket = 0;
    set_running_flag(false);
    s_app.reset();
    client_id = "";
}

bool IHProtocolInf::get_running_flag(void) {
    return f_is_run;
}

std::string IHProtocolInf::get_thread_id(void) {
    return t_id;
}

std::string IHProtocolInf::get_client_id(void) {
    return client_id;
}

/***********************************
 * Protected Function definition.
 */ 
void IHProtocolInf::load_protocols(void) {
    if ( s_proto_desp->is_there() == false ) {
        // TODO empty-protocol manager.
        LOGD("make empty-protocol manager.");
        return;
    }

    try {
        LOGD("make protocol manager according to json-script.");
        // TODO make protocol manager.
        auto libpath = s_proto_desp->get_member("protocol-libpath");
        auto proto_list = s_proto_desp->get_array_member("protocol-list");
        auto object_list = s_proto_desp->get_array_member<json_mng::CMjson>("protocol-desp");

        LOGD("libpath = %s", libpath->c_str());
        for ( auto itr = proto_list->begin(); itr != proto_list->end(); itr++ ) {
            LOGD("element=%s", (*itr)->c_str());
        }

        for ( auto itr = object_list->begin(); itr != object_list->end(); itr++ ) {
            auto name = (*itr)->get_member("name");
            auto index = (*itr)->get_member<int>("index");
            auto properties = (*itr)->get_member<json_mng::CMjson>("properties");
            LOGD("element name=%s", name->c_str());
            LOGD("element index=%d", (*index.get()));
            LOGD("element properties->msg_id=%s", properties->get_member("msg_id")->c_str());
            LOGD("element properties->length=%s", properties->get_member("length")->c_str());
        }
    }
    catch( const std::exception &e) {
        LOGERR("%s", e.what());
    }
}

void IHProtocolInf::set_running_flag(bool value) {
    f_is_run = value;
}

IHProtocolInf::SegmentsType IHProtocolInf::encapsulation(const void* msg_raw, size_t msg_size, enum_c::ServerType server_type) {
    /****
     * According to ServerType, fragment the message. & make segment-List.
     *                        - segment-list will be RawMsgType-List.
     * Segments : 1. combine with One-Head + payload + (One-Tail)
     *          : 2. Encoding(payload)
     */
    ProtocolType p_msg = std::make_shared<IProtocolInf>();
    try{
        return p_msg->pack_recursive(msg_raw, msg_size, server_type);
    }catch(const std::exception &e) {
        LOGERR("%s", e.what());
        throw std::exception(e);
    }
}

IHProtocolInf::SegmentsType IHProtocolInf::encapsulation(IHProtocolInf::ProtocolType& p_msg, enum_c::ServerType server_type) {
    /****
     * According to ServerType, fragment the message. & make segment-List.
     *                        - segment-list will be RawMsgType-List.
     * Segments : 1. combine with One-Head + payload + (One-Tail)
     *          : 2. Encoding(payload)
     */
    size_t msg_size = 0;
    const void* msg_raw = p_msg->get_payload()->get_msg_read_only(&msg_size);

    try{
        return p_msg->pack_recursive(msg_raw, msg_size, server_type);
    }catch(const std::exception &e) {
        LOGERR("%s", e.what());
        throw std::exception(e);
    }
}

IHProtocolInf::ProtocolType IHProtocolInf::decapsulation(IHProtocolInf::RawMsgType msg_raw) {
    /****
     * Classify segments from the raw-message. & extract payload & It combine with payloads.
     * Segments : 1. combine with One-Head + payload + (One-Tail)
     *          : 2. Decoding(payload)
     */
    bool res = false;
    ProtocolType p_msg = std::make_shared<IProtocolInf>();
    try{
        res = p_msg->unpack_recurcive(msg_raw->get_msg_read_only(), msg_raw->get_msg_size());
        assert(res==true);
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        throw std::exception(e);
    }
    return p_msg;
}

IHProtocolInf::AppCallerType& IHProtocolInf::get_app_instance(void) {
    return s_app;
}

int IHProtocolInf::get_sockfd(void) {
    return h_socket;
}
