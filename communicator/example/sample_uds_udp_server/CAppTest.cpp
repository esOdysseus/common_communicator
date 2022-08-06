/***
 * CAppTest.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <cassert>
#include <iostream>

#include <CAppTest.h>
#include <IProtocolInf.h>

using namespace std;

CAppTest::CAppTest(CAppTest::CommHandler handler){
    cout << "[Debug] CAppTest::constructor() is called." << endl;

    updated_peers.clear();
    h_communicator = std::forward<CommHandler>(handler);
    rcv_count = 0;
}

CAppTest::~CAppTest(void) {
    cout << "[Debug] CAppTest::deconstructor() is called." << endl;

    h_communicator.reset();
    rcv_count = 0;
    updated_peers.clear();
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
void CAppTest::cb_connected(std::string peer_app_path, std::string peer_pvd_id, rcv::ConnectionState flag, const char* from_app, const char* from_pvd) {
    std::string from_full_path="/";
    cout << "[Debug] CAppTest::cb_connected() is called (CONN: " << flag << ") for " << peer_app_path << "/" << peer_pvd_id << endl;

    if ( flag == rcv::ConnectionState::E_RESUME_UPDATE ) {
        if ( from_pvd == NULL ) {
            throw std::runtime_error("RuntimeError: we need 'from_pvd'");
        }

        from_full_path += std::string(from_pvd);
        if ( from_app != NULL ) {
            from_full_path = std::string(from_app) + from_full_path;
        }

        updated_peers[from_full_path] = std::make_pair(peer_app_path, peer_pvd_id);
    }
}

// We receved a message from client_id.
void CAppTest::cb_receive_msg_handle(std::string peer_app_path, std::string peer_pvd_path, std::shared_ptr<payload::CPayload> payload) {
    cout << "[Debug] CAppTest::cb_receive_msg_handle() is called." << endl;
    rcv_count++;

    size_t data_size = 0;
    const void* data = payload->get_payload(data_size);
    std::shared_ptr<IProtocolInf> protocol = payload->get(PBigEdian);
    cout << "************************************" << endl;
    cout << "* 0. Receive-CNT : " << rcv_count << endl;
    cout << "* 1. Peer-ID : " << peer_app_path << "/" << peer_pvd_path << endl;
    cout << "* 2. CPayload-Name : " << payload->get_name() << endl;
    cout << "* 3. payload-size : " << data_size << endl;
    cout << "* 4. payload : " << (const char*)data << endl;
    cout << "* 5. message-ID : " << protocol->get_property("msg_id") << endl;
    cout << "* 6. payload-length : " << protocol->get_property("length") << endl;
    cout << "************************************" << endl;

    // Send Message
    std::string new_msg = "Echo: " + std::string((const char*)data) + " : RcvCNT=" + std::to_string(rcv_count);
    std::shared_ptr<payload::CPayload> new_payload = h_communicator->create_payload();
    // new_payload->set_op_flag(payload::E_PAYLOAD_FLAG::E_KEEP_PAYLOAD_AFTER_TX, true);

    std::shared_ptr<IProtocolInf> new_protocol = new_payload->get(PBigEdian);
    assert( new_protocol->set_property("msg_id", protocol->get_property("msg_id")) == true );
    new_protocol->set_payload(new_msg.c_str(), new_msg.length());

    auto itr = updated_peers.find( peer_app_path + "/" + peer_pvd_path );
    if ( itr != updated_peers.end() ) {
        peer_app_path = itr->second.first;
        peer_pvd_path = itr->second.second;
        cout << "[Debug] CAppTest::cb_receive_msg_handle() Updated APP-Path to " << peer_app_path << "/" << peer_pvd_path << endl;
    }
    assert(h_communicator->send(peer_app_path, peer_pvd_path, new_payload) == true);

}
