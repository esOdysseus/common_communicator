
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
    template <typename SERVER> 
    IHProtocolInf(std::string client_addr, 
                  int socket_handler, 
                  std::shared_ptr<SERVER> &&server,
                  AppCallerType &app,
                  std::shared_ptr<CConfigProtocols> &proto_manager) {
        assert(client_addr.empty()==false);

        this->t_id = client_addr;
        this->h_socket = socket_handler;
        this->s_app = app;
        this->s_proto_desp = proto_manager;
        this->client_id = client_addr;
        
        load_protocols();
        set_running_flag(false);
    }

    ~IHProtocolInf(void);

    virtual void run(void) = 0;
    
    virtual bool destroy(void) = 0;

    bool get_running_flag(void);

    std::string get_thread_id(void);

    std::string get_client_id(void);

protected:
    void load_protocols(void);
    
    void set_running_flag(bool value);

    virtual bool set_app_call_back(void) = 0;

    virtual bool write(std::string client_id, const void* msg, size_t msg_size) = 0;

    virtual bool write_payload(std::string client_id, std::shared_ptr<payload::CPayload>&& payload) = 0;

    SegmentsType encapsulation(const void* msg_raw, size_t msg_size, enum_c::ServerType server_type);

    SegmentsType encapsulation(ProtocolType& p_msg, enum_c::ServerType server_type);

    ProtocolType decapsulation(RawMsgType msg_raw);

    AppCallerType& get_app_instance(void);

    int get_sockfd(void);

private:
    AppCallerType s_app;

    std::shared_ptr<CConfigProtocols> s_proto_desp;

    int h_socket;

    std::string t_id;

    std::string client_id;

    bool f_is_run;
};

#endif // I_HANDLER_PROTOCOL_INTERFACE_H_
