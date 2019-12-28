#ifndef C_HANDLE_ETHERNET_TP_H_
#define C_HANDLE_ETHERNET_TP_H_

#include <map>
#include <IServerInf.h>
#include <IHProtocolInf.h>
#include <BaseDatatypes.h>
#include <CRawMessage.h>
#include <json_manipulator.h>

class CHProtoBaseLan : public IHProtocolInf {
public:
    using ServerType = std::shared_ptr<IServerInf<CHProtoBaseLan>>;
    using AddressType = CRawMessage::LanAddrType;

public:
    CHProtoBaseLan(std::string client_addr, int socket_handler, 
                   ServerType &&server, AppCallerType &app, Json_DataType &json_manager);

    ~CHProtoBaseLan(void);

    void run(void) override;
    
    bool destroy(void) override;

protected:
    bool set_app_call_back(void) override;

    bool write(std::string client_id, const void* msg, size_t msg_size) override;

    bool write_payload(std::string client_id, std::shared_ptr<CPayload>&& payload) override;

private:    
    ServerType s_server;

};


#endif // C_HANDLE_ETHERNET_TP_H_
