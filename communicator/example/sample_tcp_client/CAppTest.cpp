/***
 * CAppTest.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <cassert>
#include <iostream>

#include <unistd.h>
#include <CAppTest.h>
#include <IProtocolInf.h>

using namespace std;

CAppTest::CAppTest(CAppTest::CommHandler handler){
    cout << "[Debug] CAppTest::constructor() is called." << endl;

    is_continue = false;
    h_communicator = std::forward<CommHandler>(handler);
    rcv_count = 0;
}

CAppTest::~CAppTest(void) {
    cout << "[Debug] CAppTest::deconstructor() is called." << endl;

    is_continue = false;
    if(runner.joinable() == true) {
        runner.join();
    }
    h_communicator.reset();
    rcv_count = 0;
}

// This-APP was normal-ready/quit to communicate between VMs.
void CAppTest::cb_initialization(enum_c::ProviderType provider_type, bool flag_init) {
    cout << "[Debug] CAppTest::cb_initialization() is called.(" << flag_init << ")" << endl;

    is_continue = true;
    this->runner = std::thread(&CAppTest::run_period_send, this);
}

// This-APP was exit from communication-session, abnormally.
void CAppTest::cb_abnormally_quit(const std::exception &e) {
    cout << "[Debug] CAppTest::cb_abnormally_quit() is called.(" << e.what() << ")" << endl;
}

// Client was connected.
void CAppTest::cb_connected(std::string peer_id, bool flag_connect) {
    cout << "[Debug] CAppTest::cb_connected() is called.(" << flag_connect << ")" << endl;
}

// We receved a message from peer_id.
void CAppTest::cb_receive_msg_handle(std::string peer_id, std::shared_ptr<payload::CPayload> payload) {
    cout << "[Debug] CAppTest::cb_receive_msg_handle() is called." << endl;
    rcv_count++;

    size_t data_size = 0;
    const void* data = payload->get_payload(data_size);
    std::shared_ptr<IProtocolInf> protocol = payload->get(PROTOCOL_NAME);
    cout << "************************************" << endl;
    cout << "* 0. Receive-CNT : " << rcv_count << endl;
    cout << "* 1. Peer-ID : " << peer_id << endl;
    cout << "* 2. CPayload-Name : " << payload->get_name() << endl;
    cout << "* 3. payload-size : " << data_size << endl;
    cout << "* 4. payload : " << (const char*)data << endl;
    cout << "* 5. flag : " << protocol->get_property("flag") << endl;
    cout << "* 6. state : " << protocol->get_property("state") << endl;
    cout << "* 7. msg-ID : " << protocol->get_property("msg_id") << endl;
    cout << "* 8. from-APP : " << protocol->get_property("from") << endl;
    cout << "* 9. to-APP : " << protocol->get_property("who") << endl;
    cout << "* 10. when : " << protocol->get_property("when") << endl;
    cout << "* 11. payload-length : " << protocol->get_property("length") << endl;
    cout << "************************************" << endl;
}

int CAppTest::run_period_send(void) {
    cout << "[Debug] CAppTest::run_period_send() is called." << endl;

    int send_count = 0;
    std::string msg_origin = "msg from sample-client.";

    while(is_continue) {
        // Send Message
        std::string msg = msg_origin + " : SendCNT=" + std::to_string(++send_count);
        std::shared_ptr<payload::CPayload> new_payload = h_communicator->create_payload();

        // new_payload->set_op_flag(payload::E_PAYLOAD_FLAG::E_KEEP_PAYLOAD_AFTER_TX, true);

        std::shared_ptr<IProtocolInf> new_protocol = new_payload->get(PROTOCOL_NAME);
        new_protocol->set_property("flag", 1);
        new_protocol->set_property("state", 2);
        new_protocol->set_property("msg_id", 5678);
        new_protocol->set_payload(msg.data(), msg.length());
        assert(h_communicator->send("tcp_01", new_payload) == true);

        usleep(100000);     // wait 100 ms
    }

}