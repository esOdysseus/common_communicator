#include <cassert>
#include <iostream>

#include <CAppTest.h>
#include <IProtocolInf.h>

using namespace std;

CAppTest::CAppTest(CAppTest::CommHandler handler){
    h_communicator = std::forward<CommHandler>(handler);
    cout << "[Debug] CAppTest::constructor() is called." << endl;
}

CAppTest::~CAppTest(void) {
    cout << "[Debug] CAppTest::deconstructor() is called." << endl;
}

// This-APP was normal-ready/quit to communicate between VMs.
void CAppTest::cb_initialization(enum_c::ProviderType provider_type, bool flag_init) {
    cout << "[Debug] CAppTest::cb_initialization() is called.(" << flag_init << ")" << endl;
}

// This-APP was exit from communication-session, abnormally.
void CAppTest::cb_abnormally_quit(const std::exception &e) {
    cout << "[Debug] CAppTest::cb_abnormally_quit() is called.(" << e.what() << ")" << endl;
}

// Client was connected.
void CAppTest::cb_connected(std::string client_id, bool flag_connect) {
    cout << "[Debug] CAppTest::cb_connected() is called.(" << flag_connect << ")" << endl;
}

// We receved a message from client_id.
void CAppTest::cb_receive_msg_handle(std::string client_id, std::shared_ptr<payload::CPayload> payload) {
    cout << "[Debug] CAppTest::cb_receive_msg_handle() is called." << endl;

    size_t data_size = 0;
    const void* data = payload->get_payload(data_size);
    std::shared_ptr<IProtocolInf> protocol = payload->get(PBigEdian);
    cout << "************************************" << endl;
    cout << "* 1. Client-ID : " << client_id << endl;
    cout << "* 2. CPayload-Name : " << payload->get_name() << endl;
    cout << "* 3. payload-size : " << data_size << endl;
    cout << "* 4. payload : " << (const char*)data << endl;
    cout << "* 5. message-ID : " << protocol->get_property("msg_id") << endl;
    cout << "* 6. payload-length : " << protocol->get_property("length") << endl;
    cout << "************************************" << endl;

    // Send Message
    std::shared_ptr<payload::CPayload> new_payload = h_communicator->create_payload();
    new_payload->set_op_flag(payload::E_PAYLOAD_FLAG::E_KEEP_PAYLOAD_AFTER_TX, true);

    std::shared_ptr<IProtocolInf> new_protocol = new_payload->get(PBigEdian);
    assert( new_protocol->set_property("msg_id", 1234) == true );
    new_protocol->set_payload("Hello World!", 13);
    assert(h_communicator->send(client_id, new_payload) == true);

    // Send Message
    const std::string message = "Congraturation!! Test Successful.";
    assert( h_communicator->send(client_id, message.c_str(), message.length()) == true );
}
