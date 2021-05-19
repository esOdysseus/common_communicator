/***
 * CAppTest.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef C_APP_TEST_H_
#define C_APP_TEST_H_

#include <string>
#include <IAppInf.h>

class CAppTest {
public:
    using CommHandler = std::shared_ptr<ICommunicator>;
    static constexpr const char* PROTOCOL_NAME = "CPUniversalCMD";

public:
    CAppTest(CommHandler handler);

    ~CAppTest(void);

    void cb_initialization(enum_c::ProviderType provider_type, bool flag_init);

    void cb_connected(std::string peer_id, bool flag_connect);

    void cb_receive_msg_handle(std::string peer_id, std::shared_ptr<payload::CPayload> payload);

    void cb_abnormally_quit(const std::exception &e);

private:
    CommHandler h_communicator;

    int rcv_count;

};


#endif // C_APP_TEST_H_