/***************************************************************************
 * 
 * C++ type: Concurrent multiple-Server with each specific Protocol. (IPv4)
 * 
 * *************************************************************************/

#include <iostream>
#include <unistd.h>

#include <IAppInf.h>
#include <CAppTest.h>

using namespace std;
using namespace std::placeholders;

int main(int argc, char *argv[])
{
    cout << "Test-program for check operating of UDP/TCP server." << endl;

    auto handler = create_server("TestApp01", 
                                 "CServerUDP", enum_c::ServerType::E_SERVER_UDP, 
                                 atoi(argv[2]), 
                                 argv[1]);
    CAppTest sample_App(handler);

    handler->register_initialization_handler(bind(&CAppTest::vmc_ready_cb, &sample_App, _1, _2));
    handler->register_connection_handler(bind(&CAppTest::vmc_connected_cb, &sample_App, _1, _2));
    handler->register_message_handler(bind(&CAppTest::vmc_receive_msg_payload_cb, &sample_App, _1, _2));
    handler->register_quit_handler(bind(&CAppTest::vmc_quit_cb, &sample_App, _1));

    handler->init();

    while(true) {
        sleep(1);
    }

    return 0;
}
