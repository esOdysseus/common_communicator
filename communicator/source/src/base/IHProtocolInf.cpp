
#include <cassert>
#include <list>
#include <unistd.h>

#include <logger.h>
#include <IHProtocolInf.h>

/**********************************
 * Public Function definition.
 */
IHProtocolInf::IHProtocolInf(std::string client_addr, 
                             int socket_handler, 
                             AppCallerType &app,
                             std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager) {
    assert(client_addr.empty()==false);

    this->t_id = client_addr;
    this->h_socket = socket_handler;
    this->s_app = app;
    this->s_proto_config = proto_manager;
    this->client_id = client_addr;
    
    set_running_flag(false);
}

IHProtocolInf::~IHProtocolInf(void) {
    if (h_socket != 0) {
        close(h_socket);
    }
    t_id = "destroyed";
    h_socket = 0;
    set_running_flag(false);
    s_app.reset();
    s_proto_config.reset();
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
    auto payload = s_proto_config->create_protocols_chain();
    ProtocolType protocol = payload->get(payload::CPayload::Myself_Name);
    try{
        SegmentsType& ref_segs = protocol->pack_recursive(msg_raw, msg_size, server_type);
        destroy_proto_chain(protocol);
        return ref_segs;
    }catch(const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }
}

IHProtocolInf::SegmentsType IHProtocolInf::encapsulation(IHProtocolInf::ProtocolType& protocol, enum_c::ServerType server_type) {
    /****
     * According to ServerType, fragment the message. & make segment-List.
     *                        - segment-list will be RawMsgType-List.
     * Segments : 1. combine with One-Head + payload + (One-Tail)
     *          : 2. Encoding(payload)
     */
    size_t msg_size = 0;
    const void* msg_raw = protocol->get_payload()->get_msg_read_only(&msg_size);

    try{
        return protocol->pack_recursive(msg_raw, msg_size, server_type);
    }catch(const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }
}

IHProtocolInf::ProtocolType IHProtocolInf::decapsulation(IHProtocolInf::RawMsgType msg_raw) {
    /****
     * Classify segments from the raw-message. & extract payload & It combine with payloads.
     * Segments : 1. combine with One-Head + payload + (One-Tail)
     *          : 2. Decoding(payload)
     */
    bool res = false;
    auto payload = s_proto_config->create_protocols_chain();
    ProtocolType protocol = payload->get(payload::CPayload::Myself_Name);
    try{
        size_t data_size = 0;
        const void * data = msg_raw->get_msg_read_only( &data_size );
        res = protocol->unpack_recurcive(data, data_size);
        assert(res==true);
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }

    return protocol;
}

IHProtocolInf::AppCallerType& IHProtocolInf::get_app_instance(void) {
    return s_app;
}

bool IHProtocolInf::destroy_proto_chain(ProtocolType &chain) {
    return s_proto_config->destroy_protocols_chain(chain);
}

int IHProtocolInf::get_sockfd(void) {
    return h_socket;
}
