
#ifndef IAPP_INTERFACE_H_
#define IAPP_INTERFACE_H_

#include <string>
#include <thread>
#include <functional>

#include <Enum_common.h>
#include <CReceiver.h>
// #include <CAppInternalCaller.h>

class ICommunicator;

std::shared_ptr<ICommunicator> create_communicator(std::string app_id, 
                                                   std::string server_id, 
                                                   enum_c::ServerType server_type, 
                                                   unsigned short port, 
                                                   const char* ip=NULL);

class ICommunicator {
public:
    using SendToType = std::function<bool(std::string client_id, const void* msg, size_t msg_size)>;
    using SendPayloadType = std::function<bool(std::string client_id, std::shared_ptr<CPayload>&& payload)>;
    using InitialCB_Type = CReceiver::InitialCB_Type;
    using ConnectionCB_Type = CReceiver::ConnectionCB_Type;
    using MessagePayloadCB_Type = CReceiver::MessagePayloadCB_Type;
    using QuitCB_Type = CReceiver::QuitCB_Type;

public:
    ICommunicator(std::string app_id, 
                  std::string server_id, 
                  enum_c::ServerType server_type, 
                  unsigned short port, 
                  const char* ip=NULL);

    ~ICommunicator(void);

    // Get Application-ID which contain this Communicator.
    std::string get_app_id(void);

    // Init Communicator & Start it as thread.
    void init(void);

    // It means whether done of System-Initialization for Communicator, or not.
    void register_initialization_handler(InitialCB_Type &&handler);

    // It means whether connected with Client, or not.
    void register_connection_handler(ConnectionCB_Type &&handler);

    // // It's handler will be called when received message from any-client. (Traditional)
    // void register_message_handler(MessageCB_Type &&handler);

    // It's handler will be called when received message from any-client. (with Cpayload)
    void register_message_handler(MessagePayloadCB_Type &&handler);

    // It means whether abnormally forced-quit, or not.
    void register_quit_handler(QuitCB_Type &&handler);

    // [REQ/RESP] Send response-message to Client.
    bool send(std::string client_id, std::shared_ptr<CPayload>&& payload);

    // [REQ/RESP] Send response-message to Client.
    bool send(std::string client_id, const void* msg, size_t msg_size);

protected:
    // Regist function-point for "send" member-function.
    void set_send_payload_fp(SendPayloadType &&fp);

    // Regist function-point for "send" member-function.
    void set_sendto_fp(SendToType &&fp);

    // Get group-point for Call-back functions.
    CReceiver& get_cb_handlers(void);

private:
    // Get continue-flag of server for Communicator.
    bool is_running_continue(void);

    // Thread-routin of server for Communicator.
    int run(void);

private:
    CReceiver cb_handlers;  // It contains many kinds of Call-back functions for Application.

    std::string app_id;     // Identical-Name for Application.

    std::string server_id;  // Identical-Name for Server.

    enum_c::ServerType server_type; // Type for Server.

    unsigned short port;    // Port-number for Server.

    std::string ip;         // IP-address for Server.

    SendToType m_sendto;    // function-pointer for Send-Transaction.

    SendPayloadType m_send_payload; // function-pointer for Send-Transaction with payload.

    std::thread runner;     // Thread-instance of server for Communicator.

    bool runner_continue;   // Continue-flag of server for Communicator.

};

#endif // IAPP_INTERFACE_H_