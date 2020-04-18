#ifndef C_HANDLE_ETHERNET_TP_H_
#define C_HANDLE_ETHERNET_TP_H_

#include <map>
#include <IServerInf.h>
#include <IHProtocolInf.h>
#include <BaseDatatypes.h>
#include <CRawMessage.h>

class CHProtoBaseLan : public IHProtocolInf {
public:
    using ServerType = std::shared_ptr<IServerInf>;
    using AddressType = CRawMessage::LanAddrType;

public:
    CHProtoBaseLan(ServerType &&server, AppCallerType &app, 
                   std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager);

    ~CHProtoBaseLan(void);

protected:
    bool set_app_call_back(void) override;

    bool write_payload(std::string alias, std::shared_ptr<payload::CPayload>&& payload) override;

private:
    ServerType s_server;

};


#endif // C_HANDLE_ETHERNET_TP_H_
