/***
 * ICommunicator.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <cassert>
#include <memory>
#include <map>

#include <version.h>
#include <logger.h>
#include <CPayload.h>
#include <CConfigProtocols.h>
#include <CConfigAliases.h>
#include <ICommunicator.h>
#include <ICommunicatorImpl.h>
#include <IAliasSearcher.h>
#include <CAliasSearcherImpl.h>

/*****
 * Static Function.
 */ 
using Tmap_proto = std::map<std::string /*proto_path*/, std::shared_ptr<cf_proto::CConfigProtocols>>;
static Tmap_proto  _gm_protos_;


static std::shared_ptr<cf_alias::CConfigAliases> update_get_alias( std::string& file_path ) {
    std::shared_ptr<cf_alias::CConfigAliases> res;

    try {
        auto searcher = alias::IAliasSearcher::get_instance( file_path );
        if( searcher.get() == NULL ) {
            throw std::runtime_error("Fail to get AliasSearcher.");
        }

        auto searcher_impl = std::dynamic_pointer_cast<alias::CAliasSearcherImpl>(searcher);
        if( searcher_impl.get() == NULL ) {
            throw std::runtime_error("Fail to get AliasSearcherImpl.");
        }

        res = searcher_impl->get_config_alias();
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return res;
}

static std::shared_ptr<cf_proto::CConfigProtocols> update_get_proto( std::string& file_path ) {
    Tmap_proto::iterator iter;

    try {
        iter = _gm_protos_.find(file_path);

        if( iter == _gm_protos_.end() ) {
            auto res = _gm_protos_.insert( {file_path, std::make_shared<cf_proto::CConfigProtocols>(file_path.data())} );
            if( res.second != true ) {
                std::string err = "Key is duplicated. (" + file_path + ")";
                throw std::logic_error(err);
            }
            iter = res.first;
        }
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }
    return iter->second;
}


/*****
 * Public Member Function.
 */ 
// Constructor API for Dynamic-Auto parsed Provider.
ICommunicator::ICommunicator(std::string app_id, 
                             std::string provider_id, 
                             std::string alias_file_path,
                             std::string protocol_file_path,
                             enum_c::ProviderMode mode) {
    try {
        auto alias_config = update_get_alias( alias_file_path );
        auto proto_config = update_get_proto( protocol_file_path );

#if __cplusplus > 201103L
        _m_impl_ = std::make_unique<ICommunicatorImpl>(std::move(app_id), std::move(provider_id), alias_config, proto_config, mode);
#else
        _m_impl_ = std::make_shared<ICommunicatorImpl>(std::move(app_id), std::move(provider_id), alias_config, proto_config, mode);
#endif
    }
    catch ( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

// Constructor API for Static-Defined Provider. (Only for Unregisted Transaction-Communication)
ICommunicator::ICommunicator(std::string app_id, 
                             std::string provider_id, 
                             std::string alias_file_path,
                             std::string protocol_file_path,
                             enum_c::ProviderType provider_type, 
                             unsigned short port,
                             const char* ip,
                             enum_c::ProviderMode mode) {
    try {
        auto alias_config = update_get_alias( alias_file_path );
        auto proto_config = update_get_proto( protocol_file_path );
#if __cplusplus > 201103L
        _m_impl_ = std::make_unique<ICommunicatorImpl>(std::move(app_id), std::move(provider_id), alias_config, proto_config, provider_type, port, ip, mode);
#else
        _m_impl_ = std::make_shared<ICommunicatorImpl>(std::move(app_id), std::move(provider_id), alias_config, proto_config, provider_type, port, ip, mode);
#endif
    }
    catch ( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

ICommunicator::~ICommunicator(void) {
    quit();     // thread quit
    _m_impl_.reset();
}

std::string ICommunicator::get_app_id(void) { 
    return _m_impl_->get_app_id();
}

std::string ICommunicator::get_provider_id(void) { 
    return _m_impl_->get_provider_id();
}

std::string ICommunicator::get_version(void) {
    return STRING_OF_COMMON_API_VERSION;
}

void ICommunicator::init(void) {
    _m_impl_->init();
}

void ICommunicator::quit(void) {
    _m_impl_->quit();
}

void ICommunicator::register_initialization_handler(InitialCB_Type &&handler) {
    _m_impl_->register_initialization_handler( std::forward<InitialCB_Type>(handler) );
}

void ICommunicator::register_connection_handler(ConnectionCB_Type &&handler) {
    _m_impl_->register_connection_handler( std::forward<ConnectionCB_Type>(handler) );
}

void ICommunicator::register_message_handler(MessagePayloadCB_Type &&handler) {
    _m_impl_->register_message_handler( std::forward<MessagePayloadCB_Type>(handler) );
}

void ICommunicator::register_unintended_quit_handler(QuitCB_Type &&handler) {
    _m_impl_->register_unintended_quit_handler( std::forward<QuitCB_Type>(handler) );
}

std::shared_ptr<payload::CPayload> ICommunicator::create_payload(void) {
    return _m_impl_->create_payload();
}

bool ICommunicator::send(std::string app_path, std::string pvd_path, 
                         std::shared_ptr<payload::CPayload>&& payload) {
    return _m_impl_->send(std::move(app_path), std::move(pvd_path), std::move(payload));
}

bool ICommunicator::send(std::string app_path, std::string pvd_path, 
                         std::shared_ptr<payload::CPayload>& payload) {
    return _m_impl_->send(std::move(app_path), std::move(pvd_path), payload);
}

bool ICommunicator::send(std::string app_path, std::string pvd_path, 
                         const void* msg, size_t msg_size) {
    try {
        auto payload = create_payload();
        assert( payload->set_payload(msg, msg_size) == true );
        return send( std::move(app_path), std::move(pvd_path), payload );
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return false;
}

bool ICommunicator::connect_try(std::string &peer_ip, uint16_t peer_port, std::string& app_path, std::string &pvd_id) {
    return _m_impl_->connect_try( peer_ip, peer_port, app_path, pvd_id );
}

bool ICommunicator::connect_try(std::string && app_path, std::string && pvd_id) {
    return _m_impl_->connect_try( std::forward<std::string>(app_path), std::forward<std::string>(pvd_id) );
}

void ICommunicator::disconnect(std::string & app_path, std::string & pvd_id) {
    _m_impl_->disconnect( app_path, pvd_id );
}

void ICommunicator::disconnect(std::string && app_path, std::string && pvd_id) {
    _m_impl_->disconnect( std::forward<std::string>(app_path), std::forward<std::string>(pvd_id) );
}
