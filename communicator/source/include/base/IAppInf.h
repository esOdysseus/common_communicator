
/***
 * IAppInf.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef IAPP_INTERFACE_H_
#define IAPP_INTERFACE_H_

#include <string>
#include <thread>
#include <map>
#include <memory>
#include <functional>

#include <Enum_common.h>
#include <CReceiver.h>
#include <CAppInternalCaller.h>
#include <CConfigProtocols.h>
#include <CConfigAliases.h>

#include <IServerInf.h>

class ICommunicator;

std::shared_ptr<ICommunicator> create_communicator(std::string app_id, 
                                                   std::string provider_id, 
                                                   enum_c::ProviderType provider_type, 
                                                   unsigned short port=0, 
                                                   const char* ip=NULL,
                                                   enum_c::ProviderMode mode=enum_c::ProviderMode::E_PVDM_BOTH,
                                                   const char* protocol_desp_path=NULL,
                                                   const char* alias_desp_path=NULL);

class ICommunicator {
public:
    using SendPayloadType = CAppInternalCaller::SendPayloadType;
    using SharedThisType = std::enable_shared_from_this<ICommunicator>;
    using InitialCB_Type = CReceiver::InitialCB_Type;
    using ConnectionCB_Type = CReceiver::ConnectionCB_Type;
    using MessagePayloadCB_Type = CReceiver::MessagePayloadCB_Type;
    using QuitCB_Type = CReceiver::QuitCB_Type;
    using ProtocolProperties = std::shared_ptr<std::map<std::string, std::string>>;

public:
    ICommunicator(std::string app_id, 
                  std::string provider_id, 
                  enum_c::ProviderType provider_type, 
                  std::shared_ptr<cf_proto::CConfigProtocols> &proto_config,
                  std::shared_ptr<cf_alias::CConfigAliases> &alias_config,
                  unsigned short port=0,
                  const char* ip=NULL,
                  enum_c::ProviderMode mode=enum_c::ProviderMode::E_PVDM_BOTH);

    ~ICommunicator(void);

    // Get Application-ID which contain this Communicator.
    std::string get_app_id(void);

    // Get Version-infomation of Common-API.
    std::string get_version(void);

    /******************************
     * Communicator-Base API
     ******************************/
    // >> Server/Client-side
    //   - Precondition : None
    //   - Practice : System initialize to communicate.
    //   - Postcondition : Handler registered by cb_register_initialization_handler() will be called.
    void init(void);     // Mandatory

    // >> Server/Client-side
    //   - Precondition : init() was called
    //   - Practice : System destroy against communicate.
    //   - Postcondition : Handler registered by cb_register_initialization_handler() will be called.
    void quit(void);     // Mandatory

    // >> Server/Client-side
    //   - Precondition : None
    //   - Practice : Register handler for checking System-init or quit INFO, normally.
    //   - Postcondition : When Self trig init()/quit(), the handler will be called.
    void register_initialization_handler(InitialCB_Type &&handler);     // Mandatory

    // >> Server/Client-side
    //   - Precondition : None
    //   - Practice : Register handler for checking System-quit INFO, abnormally.
    //   - Postcondition : When the system was forced-quit abnormally in run-time, the handler will be called.
    void register_unintended_quit_handler(QuitCB_Type &&handler);

    // >> Server/Client-side
    //   - Precondition : None
    //   - Practice : Create CPayload with protocol-chain instance in internal.
    //   - Postcondition : you can get CPayload instance with protocol-chain.
    std::shared_ptr<payload::CPayload> create_payload(void);


    /******************************
     * Transaction API
     ******************************/
    /**
     * Server(=Cloud)/Client common
     */
    // >> Server-side
    //   - Precondition: None
    //   - Practice : Register handler for receiving Connection-State INFO. from Client.
    //   - Postcondition: If Server trig resume() function, and Client did try connect()/disconnect(), 
    //                    then the registered Handler will be called.
    // >> Client-side
    //   - Precondition: None
    //   - Practice : Register handler for receiving Connection-State INFO. from Cloud/Server/Service-Provider.
    //   - Postcondition: When Client try connect()/disconnect(), the Handler will be called.
    void register_connection_handler(ConnectionCB_Type &&handler);

    // >> Server-side
    //   - Precondition : None
    //   - Practice : Register handler for handling authentication of corresponder.
    //   - Postcondition : When connection_handling was successful.
    //                     And, if Client request using send_athentication(),
    //                     then the handler will be called.
    // >> Client-side
    //   - Precondition : None
    //   - Practice : Register handler for handling authentication of corresponder.
    //   - Postcondition : When connection_handling was successful.
    //                     And, if Cloud,Server,Service-Provider reply using send_athentication(),
    //                     then the handler will be called.
    // void register_athentication_handler(void);

    // >> Server-side
    //   - Precondition : None
    //   - Practice : Register handler for receiving Communicate-available-State INFO.
    //   - Postcondition : When Server trig resume(), The handler will be called.
    // >> Client-side
    //   - Precondition : None
    //   - Practice : Register handler for receiving Communicate-available-State INFO.
    //   - Postcondition : When athentication_handling was successful.
    //                     the registered Handler will be called.
    // void register_available_handler(void);    // Mandatory

    // >> Server-side
    //   - Precondition : None
    //   - Practice : [REQ/RESP] If Client trig send(), 
    //                           then Receive request-message from Client.
    // >> Client-side
    //   - Precondition : None
    //   - Practice : [REQ/RESP][PUB/SUB] If Server trig send()/notify(), 
    //                           then Receive response/notification-message from Server.
    void register_message_handler(MessagePayloadCB_Type &&handler);      // Mandatory

    // >> Server-side (Optional)
    //   - Precondition : Handler registered by cb_register_athentication_handler() was called. 
    //   - Practice : Try to do Hand-shaking with regard to authentication.
    //   - Postcondition : None
    // >> Client-side (Optional)
    //   - Precondition : Handler registered by cb_register_connection_handler() was called.
    //   - Practice : Try to do Hand-shaking with regard to authentication.
    //   - Postcondition : Handler registered by cb_register_athentication_handler() will be called. 
    // void send_athentication(void);

    // >> Server-side
    //   - Precondition : Handler registered by cb_register_message_handler() was called.
    //   - Practice : [REQ/RESP] Reply response-message to Client.
    //   - Postcondition : None
    // >> Client-side
    //   - Precondition : Handler registered by cb_register_available_handler() was called.
    //   - Practice : [REQ/RESP] Send request-message to Server.
    //   - Postcondition : If Server reply with response-message,
    //                     then Handler registered by cb_register_message_handler() will be called.
    bool send(std::string alias, std::shared_ptr<payload::CPayload>&& payload);      // Mandatory

    // [REQ/RESP] Send response-message to Client.
    bool send(std::string alias, std::shared_ptr<payload::CPayload>& payload);      // Mandatory

    // [REQ/RESP] Send response-message to Client.
    bool send(std::string alias, const void* msg, size_t msg_size);                 // Mandatory

    /**
     * Client-Side
     */
    //   - Precondition : Handler registered by cb_register_initialization_handler() was called.
    //   - Practice : connect to Cloud/Server/Service-Provider.
    //   - Postcondition : Handler registered by cb_register_connection_handler() will be called.
    //                   : If authentication mode is DISABLE,
    //                     then Handler registered by cb_register_available_handler() will be called.
    bool connect_try(std::string &peer_ip, uint16_t peer_port, std::string &new_alias_name);  // Mandatory

    // connect to peer that is pre-named as 'alias' variable.
    bool connect_try(std::string && alias);  // Mandatory

    //   - Precondition : connect() was called
    //   - Practice : disconnect from Cloud/Server/Service-Provider.
    //   - Postcondition : Handler registered by cb_register_available_handler() will be called.
    //                   : Handler registered by cb_register_connection_handler() will be called.
    void disconnect(std::string & alias);   // Mandatory

    void disconnect(std::string && alias);   // Mandatory

    //   - Precondition : Handler registered by cb_register_available_handler() was called.
    //   - Practice : [PUB/SUB] Delare Event-Accepting to Service-Provider.
    //   - Postcondition : Handler registered by cb_register_subscription_handler() will be called.
    // void subscribe(void);   // Conditional-Mandatory

    //   - Precondition : Handler registered by cb_register_available_handler() was called.
    //   - Practice : [PUB/SUB] Delare Event-Rejecting to Service-Provider.
    //   - Postcondition : Handler registered by cb_register_subscription_handler() will be called.
    // void unsubscribe(void);  // Conditional-Mandatory

    //   - Precondition : None
    //   - Practice : [PUB/SUB] Receiving ACK-message of Service-Provider correspond to subscribe()/unsubscribe().
    // void cb_register_subscription_handler(void);

    /**
     * Server-Side
     */
    //   - Precondition : Handler registered by cb_register_initialization_handler() was called.
    //   - Practice : Cloud,Server,Service start.
    //   - Postcondition : Handler registered by cb_register_available_handler() will be called.
    // void resume(void);   // Mandatory

    //   - Precondition : Handler registered by cb_register_initialization_handler() was called.
    //   - Practice : Cloud,Server,Service temporary-stop.
    //   - Postcondition : Handler registered by cb_register_available_handler() will be called.
    // void suspend(void);  // Mandatory

    //   - Precondition : Handler registered by cb_register_available_handler() was called.
    //   - Practice : [PUB/SUB] Send Notification-message to Subscribers.
    // void notify(void);   // Conditional-Mandatory

    /******************************
     * Discovery API
     ******************************/

protected:
    // Regist function-point for "send" member-function.
    void set_send_payload_fp(SendPayloadType &&fp);

    // Get group-point for Call-back functions.
    CReceiver& get_cb_handlers(void);

private:
    void clear(void);

    void validation_check(void);

    // Get continue-flag of provider for Communicator.
    bool is_running_continue(void);

    // Thread-routin of provider for Communicator.
    int run(void);

    void force_exit_thread(std::thread &h_thread);

private:
    CReceiver cb_handlers;  // It contains many kinds of Call-back functions for Application.

    std::string app_id;     // Identical-Name for Application.

    std::string provider_id;  // Identical-Name for Provider.

    enum_c::ProviderType provider_type; // Type for Provider.

    unsigned short port;    // Port-number for Provider.

    std::string ip;         // IP-address for Provider.

    enum_c::ProviderMode mode;    // Provider mode. (Both=default/Server/Client)

    SendPayloadType m_send_payload; // function-pointer for Send-Transaction with payload.

    std::thread runner;     // Thread-instance of provider for Communicator.

    bool runner_continue;   // Continue-flag of provider for Communicator.

    std::shared_ptr<cf_proto::CConfigProtocols> proto_config;   // protocol-configration.

    std::shared_ptr<cf_alias::CConfigAliases> alias_config;     // alias-configuration.

    std::shared_ptr<IServerInf> h_pvd;  // Handler of provider instance.

};

#endif // IAPP_INTERFACE_H_
