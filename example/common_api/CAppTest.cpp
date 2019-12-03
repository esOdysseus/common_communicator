#include <cassert>
#include <iostream>

#include <CAppTest.h>
#include <CPBigEndian.h>

using namespace std;

CAppTest::CAppTest(CAppTest::ServerHandler handler){
    h_server = std::forward<ServerHandler>(handler);
    cout << "[Debug] CAppTest::constructor() is called." << endl;
}

CAppTest::~CAppTest(void) {
    cout << "[Debug] CAppTest::deconstructor() is called." << endl;
}

// This-APP was ready to communicate between VMs.
void CAppTest::vmc_ready_cb(enum_c::ServerType server_type, bool flag_init) {
    cout << "[Debug] CAppTest::vmc_ready_cb() is called.(" << flag_init << ")" << endl;
}

// This-APP was exit from communication-session.
void CAppTest::vmc_quit_cb(bool flag_force_exit) {
    cout << "[Debug] CAppTest::vmc_exit_cb() is called.(" << flag_force_exit << ")" << endl;
}

// Client was connected.
void CAppTest::vmc_connected_cb(std::string client_id, bool flag_connect) {
    cout << "[Debug] CAppTest::vmc_connected_cb() is called.(" << flag_connect << ")" << endl;
}

// We receved a message from client_id.
void CAppTest::vmc_receive_msg_payload_cb(std::string client_id, std::shared_ptr<CPayload> payload) {
    cout << "[Debug] CAppTest::vmc_receive_msg_payload_cb() is called." << endl;

    auto data = payload->get_payload();
    std::shared_ptr<CPBigEndian> protocol = payload->get<CPBigEndian>();
    cout << "************************************" << endl;
    cout << "* 1. Client-ID : " << client_id << endl;
    cout << "* 2. CPayload-Name : " << payload->get_name() << endl;
    cout << "* 3. payload-size : " << data->get_msg_size() << endl;
    cout << "* 4. payload : " << (const char*)data->get_msg_read_only() << endl;
    cout << "* 5. message-ID : " << protocol->get_msg_id() << endl;
    cout << "************************************" << endl;

    std::shared_ptr<CPBigEndian> send_msg = std::make_shared<CPBigEndian>();
    send_msg->set_msg_id(32);
    send_msg->set_payload("Hellow World!!!", 16);
    assert(h_server->send(client_id, send_msg) == true);
    // assert(h_server->send(client_id, "Hellow World!!!", 16) == true);
}
