/***
 * IHProtocolInf.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef I_HANDLER_PROTOCOL_INTERFACE_H_
#define I_HANDLER_PROTOCOL_INTERFACE_H_

#include <list>
#include <cassert>
#include <string>
#include <memory>
#include <functional>

#include <CRawMessage.h>
#include <Enum_common.h>
#include <BaseDatatypes.h>
#include <CPayload.h>
#include <IProtocolInf.h>
#include <CConfigProtocols.h>
#include <CThreadPool.h>


class IPVDInf;

class IHProtocolInf {
public:
    using RawMsgType = dtype_b::RawMsgType;
    using MsgType = dtype_b::MsgType;
    using SegmentsType = dtype_b::SegmentsType;
    using AppCallerType = dtype_b::AppCallerType;
    using ProtocolType = std::shared_ptr<IProtocolInf>;
    using TfuncUpdator = std::function<void(std::string&&)>;

public:
    IHProtocolInf(std::shared_ptr<IPVDInf>& provider, AppCallerType &app,
                  std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager);

    ~IHProtocolInf(void);

    bool handle_initialization(enum_c::ProviderType pvd_type, bool flag);

    void handle_connection(std::string app_path, std::string pvd_path, rcv::ConnectionState flag, const char* from_app=NULL, const char* from_pvd=NULL);

    bool handle_protocol_chain(RawMsgType msg_raw, std::shared_ptr<TfuncUpdator> update_peer_alias=nullptr);

    bool handle_unintended_quit(const std::exception &e);

protected:
    virtual bool set_app_call_back(void) = 0;

    virtual bool write_payload(std::string app_path, std::string pvd_path, std::shared_ptr<payload::CPayload>&& payload) = 0;

    SegmentsType encapsulation(ProtocolType& protocol, enum_c::ProviderType provider_type,
                               std::string &&from_app);

    ProtocolType decapsulation(RawMsgType msg_raw);

    AppCallerType& get_app_instance(void);

    bool destroy_proto_chain(ProtocolType &chain);

protected:
    std::shared_ptr<IPVDInf> m_provider;

private:
    AppCallerType s_app;

    std::shared_ptr<CThreadPool<RawMsgType>> _rxthr_pool_;

    std::shared_ptr<cf_proto::CConfigProtocols> s_proto_config;

};

#endif // I_HANDLER_PROTOCOL_INTERFACE_H_
