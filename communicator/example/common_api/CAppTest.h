#ifndef C_APP_TEST_H_
#define C_APP_TEST_H_

#include <string>
#include <IAppInf.h>

class CAppTest {
public:
    using ServerHandler = std::shared_ptr<ICommunicator>;
    static constexpr char* PBigEdian = "CPBigEndian";
    static constexpr char* PLittleEndian = "CPLittleEndian";

public:
    CAppTest(ServerHandler handler);

    ~CAppTest(void);

    void cb_initialization(enum_c::ServerType server_type, bool flag_init);

    void cb_connected(std::string client_id, bool flag_connect);

    void cb_receive_msg_handle(std::string client_id, std::shared_ptr<payload::CPayload> payload);

    void cb_abnormally_quit(const std::exception &e);

private:
    ServerHandler h_server;

};


#endif // C_APP_TEST_H_