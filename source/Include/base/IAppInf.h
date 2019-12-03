
#ifndef IAPP_INTERFACE_H_
#define IAPP_INTERFACE_H_

#include <string>
#include <thread>
#include <functional>

#include <Enum_common.h>
#include <CReceiver.h>
#include <CAppInternalCaller.h>

class ICommunicator;

std::shared_ptr<ICommunicator> create_server(std::string app_id, 
                                             std::string server_id, 
                                             enum_c::ServerType server_type, 
                                             unsigned short port, 
                                             const char* ip=NULL);

class ICommunicator {
public:
    using SendToType = CAppInternalCaller::SendToType;
    using SendPayloadType = CAppInternalCaller::SendPayloadType;
    using SharedThisType = std::enable_shared_from_this<ICommunicator>;
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
    // void quit(void);     // Mandatory

    // >> Server/Client-side
    //   - Precondition : None
    //   - Practice : Register handler for checking System-Ready INFO.
    //   - Postcondition : When Self trig init()/exit(), the handler will be called.
    void register_initialization_handler(InitialCB_Type &&handler);     // Mandatory

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

    // >> Server-side (Optional)
    //   - Precondition : None
    //   - Practice : Register handler for handling authentication of corresponder.
    //   - Postcondition : When connection_handling was successful.
    //                     And, if Client request using send_athentication(),
    //                     then the handler will be called.
    // >> Client-side (Optional)
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

    // It means whether abnormally forced-quit, or not.
    void register_quit_handler(QuitCB_Type &&handler);

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
    bool send(std::string client_id, std::shared_ptr<CPayload>&& payload);

    // [REQ/RESP] Send response-message to Client.
    bool send(std::string client_id, const void* msg, size_t msg_size);

    /**
     * Client-Side
     */
    //   - Precondition : Handler registered by cb_register_initialization_handler() was called.
    //   - Practice : connect to Cloud/Server/Service-Provider.
    //   - Postcondition : Handler registered by cb_register_connection_handler() will be called.
    //                   : If authentication mode is DISABLE,
    //                     then Handler registered by cb_register_available_handler() will be called.
    // void connect(void);  // Mandatory

    //   - Precondition : connect() was called
    //   - Practice : disconnect from Cloud/Server/Service-Provider.
    //   - Postcondition : Handler registered by cb_register_available_handler() will be called.
    //                   : Handler registered by cb_register_connection_handler() will be called.
    // void disconnect(void);   // Mandatory

    //   - Precondition : Handler registered by cb_register_available_handler() was called.
    //   - Practice : [PUB/SUB] Delare Event-Accepting to Service-Provider.
    //   - Postcondition : Handler registered by cb_register_subscription_handler() will be called.
    // void subscribe(void);

    //   - Precondition : Handler registered by cb_register_available_handler() was called.
    //   - Practice : [PUB/SUB] Delare Event-Rejecting to Service-Provider.
    //   - Postcondition : Handler registered by cb_register_subscription_handler() will be called.
    // void unsubscribe(void);

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
    // void notify(void);

    /******************************
     * Discovery API
     ******************************/

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