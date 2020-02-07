
#ifndef I_HANDLER_PROTOCOL_INTERFACE_H_
#define I_HANDLER_PROTOCOL_INTERFACE_H_

#include <list>
#include <cassert>
#include <string>
#include <memory>

#include <IAppInf.h>
#include <CRawMessage.h>
#include <Enum_common.h>
#include <BaseDatatypes.h>
#include <CPayload.h>
#include <IProtocolInf.h>
#include <CConfigProtocols.h>

class IHProtocolInf {
public:
    using RawMsgType = dtype_b::RawMsgType;
    using MsgType = dtype_b::MsgType;
    using SegmentsType = dtype_b::SegmentsType;
    using AppCallerType = dtype_b::AppCallerType;
    using AppType = dtype_b::AppType;
    using ProtocolType = std::shared_ptr<IProtocolInf>;

public:

    IHProtocolInf(AppCallerType &app,
                  std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager);

    ~IHProtocolInf(void);

    bool handle_initialization(enum_c::ProviderType pvd_type, bool flag);

    bool handle_connection(std::string alias, bool flag);

    bool handle_protocol_chain(RawMsgType msg_raw);

    bool handle_unintended_quit(const std::exception &e);

protected:
    virtual bool set_app_call_back(void) = 0;

    virtual bool write_payload(std::string alias, std::shared_ptr<payload::CPayload>&& payload) = 0;

    SegmentsType encapsulation(ProtocolType& protocol, enum_c::ProviderType provider_type);

    ProtocolType decapsulation(RawMsgType msg_raw);

    AppCallerType& get_app_instance(void);

    bool destroy_proto_chain(ProtocolType &chain);

private:
    AppCallerType s_app;

    std::shared_ptr<cf_proto::CConfigProtocols> s_proto_config;

};

#endif // I_HANDLER_PROTOCOL_INTERFACE_H_
