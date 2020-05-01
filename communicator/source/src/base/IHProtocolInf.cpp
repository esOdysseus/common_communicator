/***
 * IHProtocolInf.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <cassert>
#include <list>
#include <unistd.h>

#include <logger.h>
#include <IHProtocolInf.h>

/**********************************
 * Public Function definition.
 */
IHProtocolInf::IHProtocolInf(AppCallerType &app,
                             std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager) {
    this->s_app = app;
    this->s_proto_config = proto_manager;
}

IHProtocolInf::~IHProtocolInf(void) {
    s_app.reset();
    s_proto_config.reset();
}

/***********************************
 * Protected Function definition.
 */ 
IHProtocolInf::SegmentsType IHProtocolInf::encapsulation(IHProtocolInf::ProtocolType& protocol, 
                                                         enum_c::ProviderType provider_type) {
    /****
     * According to ServerType, fragment the message. & make segment-List.
     *                        - segment-list will be RawMsgType-List.
     * Segments : 1. combine with One-Head + payload + (One-Tail)
     *          : 2. Encoding(payload)
     */
    size_t msg_size = 0;
    const void* msg_raw = protocol->get_payload()->get_msg_read_only(&msg_size);

    try{
        return protocol->pack_recursive(msg_raw, msg_size, provider_type);
    }catch(const std::exception &e) {
        LOGERR("%s", e.what());
        throw ;
    }
}

IHProtocolInf::ProtocolType IHProtocolInf::decapsulation(IHProtocolInf::RawMsgType msg_raw) {
    /****
     * Classify segments from the raw-message. & extract payload & It combine with payloads.
     * Segments : 1. combine with One-Head + payload + (One-Tail)
     *          : 2. Decoding(payload)
     */
    bool res = false;
    size_t data_size = 0;
    const void * data = NULL;
    std::shared_ptr<payload::CPayload> payload;
    ProtocolType protocol;

    try{
        payload = s_proto_config->create_protocols_chain();
        protocol = payload->get(payload::CPayload::Myself_Name);

        data = msg_raw->get_msg_read_only( &data_size );
        res = protocol->unpack_recurcive(data, data_size);
        assert(res==true);
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
        throw ;
    }

    return protocol;
}

IHProtocolInf::AppCallerType& IHProtocolInf::get_app_instance(void) {
    return s_app;
}

bool IHProtocolInf::handle_initialization(enum_c::ProviderType pvd_type, bool flag) {
    try {
        AppCallerType& app = get_app_instance();
        assert(app.get() != NULL);

        app->get_cb_handlers().cb_initialization_handle(pvd_type, flag);
        return true;
    }
    catch (const std::exception &e ) {
        LOGERR("%s", e.what());
        throw ;
    }
    return false;
}

void IHProtocolInf::handle_connection(std::string alias, bool flag) {
    try {
        AppCallerType& app = get_app_instance();
        assert(app.get() != NULL);

        app->get_cb_handlers().cb_connection_handle( alias, flag );
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        throw ;
    }
}

bool IHProtocolInf::handle_protocol_chain(RawMsgType msg_raw) {
    try {
        AppCallerType& app = get_app_instance();
        assert(app.get() != NULL);

        // message parsing with regard to PROTOCOL.
        ProtocolType p_msg = decapsulation(msg_raw);
        if(p_msg->is_empty() == false) {
            app->get_cb_handlers().cb_message_payload_handle(msg_raw->get_source_alias(),
                                                            p_msg);  // trig app-function.
        }
        destroy_proto_chain(p_msg);
        return true;
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        throw ;
    }

    return false;
}

bool IHProtocolInf::handle_unintended_quit(const std::exception &e) {
    try {
        AppCallerType& app = get_app_instance();
        assert(app.get() != NULL);

        app->get_cb_handlers().cb_quit_handle(e);
        return true;
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        throw ;
    }
    return false;
}

bool IHProtocolInf::destroy_proto_chain(ProtocolType &chain) {
    return s_proto_config->destroy_protocols_chain(chain);
}
