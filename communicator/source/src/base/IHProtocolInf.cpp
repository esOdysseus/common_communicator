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
#include <IPVDInf.h>

/**********************************
 * Public Function definition.
 */
IHProtocolInf::IHProtocolInf(std::shared_ptr<IPVDInf>& provider, AppCallerType &app,
                             std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager) {
    this->m_provider = std::forward<std::shared_ptr<IPVDInf>>(provider);
    this->s_app = app;
    this->_rxthr_pool_ = std::make_shared<CThreadPool<RawMsgType>>(1);
    this->s_proto_config = proto_manager;
}

IHProtocolInf::~IHProtocolInf(void) {
    s_app.reset();
    _rxthr_pool_.reset();
    s_proto_config.reset();
    m_provider.reset();
}

/***********************************
 * Protected Function definition.
 */ 
IHProtocolInf::SegmentsType IHProtocolInf::encapsulation(IHProtocolInf::ProtocolType& protocol, 
                                                         enum_c::ProviderType provider_type,
                                                         std::string &&from_app) {
    /****
     * According to ServerType, fragment the message. & make segment-List.
     *                        - segment-list will be RawMsgType-List.
     * Segments : 1. combine with One-Head + payload + (One-Tail)
     *          : 2. Encoding(payload)
     */
    size_t msg_size = 0;
    const void* msg_raw = protocol->get_payload()->get_msg_read_only(&msg_size);

    try{
        return protocol->pack_recursive(msg_raw, msg_size, provider_type, 
                                        std::forward<std::string>(from_app));
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
        throw e;
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
        throw e;
    }
    return false;
}

void IHProtocolInf::handle_connection(std::string app_path, std::string pvd_path, bool flag) {
    try {
        AppCallerType& app = get_app_instance();
        assert(app.get() != NULL);

        if( flag == false ) {
            m_provider->unregist_connected_peer( cf_alias::IAlias::make_full_path(app_path, pvd_path) );
        }
        app->get_cb_handlers().cb_connection_handle( app_path, pvd_path, flag );
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }
}

bool IHProtocolInf::handle_protocol_chain(RawMsgType msg_raw, std::shared_ptr<TfuncUpdator> update_peer_alias) {

    auto lamda_func = [this, update_peer_alias](RawMsgType msg_raw)->void {
        std::shared_ptr<payload::CPayload> payload;
        ProtocolType protocol;
        size_t msg_size = 0;
        size_t data_size = 0;
        AppCallerType& app = get_app_instance();
        const void* data = msg_raw->get_msg_read_only(&data_size);
        assert(app.get() != NULL);
        assert(data != NULL);
        assert(data_size > 0);

        payload = s_proto_config->create_protocols_chain();
        protocol = payload->get(payload::CPayload::Myself_Name);

        while( data_size > 0 && (msg_size = protocol->get_msg_size(data, data_size)) > 0 ) {
            // message parsing with regard to PROTOCOL.
            assert( protocol->unpack_recurcive(data, msg_size) == true );
            if( update_peer_alias != nullptr ) {
                (*update_peer_alias)( protocol->who_is_owner() );
            }

            // if(protocol->is_empty() == false) {
            app->get_cb_handlers().cb_message_payload_handle(msg_raw->get_source_app(), msg_raw->get_source_pvd(), 
                                                             protocol);  // trig app-function.
            // }

            // move to next msg
            protocol->clean_data();   // re-initialize proto_chain.
            data = ((uint8_t*)data) + msg_size;
            data_size -= msg_size;
        }

        destroy_proto_chain(protocol);
    };

    try {
        _rxthr_pool_->run_thread(lamda_func, false, msg_raw);
        return true;
    }
    catch (const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
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
        throw e;
    }
    return false;
}

bool IHProtocolInf::destroy_proto_chain(ProtocolType &chain) {
    return s_proto_config->destroy_protocols_chain(chain);
}
