#ifndef C_APP_TEST_H_
#define C_APP_TEST_H_

#include <string>
#include <IAppInf.h>

class CAppTest {
public:
    using ServerHandler = std::shared_ptr<ICommunicator>;

public:
    CAppTest(ServerHandler handler);

    ~CAppTest(void);

    void vmc_ready_cb(enum_c::ServerType server_type, bool flag_init);

    void vmc_connected_cb(std::string client_id, bool flag_connect);

    void vmc_receive_msg_payload_cb(std::string client_id, std::shared_ptr<CPayload> payload);

    void vmc_quit_cb(bool flag_force_exit);

private:
    ServerHandler h_server;

};


#endif // C_APP_TEST_H_