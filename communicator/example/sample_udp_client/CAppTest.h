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
#include <thread>

#include <ICommunicator.h>

class CAppTest {
public:
    using CommHandler = std::shared_ptr<ICommunicator>;
    static constexpr const char* PBigEdian = "CPBigEndian";
    static constexpr const char* PLittleEndian = "CPLittleEndian";

public:
    CAppTest(CommHandler handler);

    ~CAppTest(void);

    void cb_initialization(enum_c::ProviderType provider_type, bool flag_init);

    void cb_connected(std::string peer_app_path, std::string peer_pvd_id, rcv::ConnectionState flag, const char* from_app, const char* from_pvd);

    void cb_receive_msg_handle(std::string peer_app_path, std::string peer_pvd_path, std::shared_ptr<payload::CPayload> payload);

    void cb_abnormally_quit(const std::exception &e);

private:
    int run_period_send(void);

private:
    CommHandler h_communicator;

    int rcv_count;

    bool is_continue;
    
    std::thread runner;

};


#endif // C_APP_TEST_H_