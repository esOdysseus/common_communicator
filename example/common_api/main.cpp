/***************************************************************************
 * 
 * C++ type: Concurrent multiple-Server with each specific Protocol. (IPv4)
 * 
 * *************************************************************************/

#include <iostream>
#include <unistd.h>

#include <IAppInf.h>
#include <CAppTest.h>

using namespace std::placeholders;

int main(int argc, char *argv[])
{
    std::cout << "Test-program for check operating of UDP/TCP server." << std::endl;

    auto handler = create_communicator("TestApp01", 
                                       "CServerUDP", enum_c::ServerType::E_SERVER_UDP, 
                                       atoi(argv[2]), 
                                       argv[1]);
    CAppTest sample_App(handler);

    handler->register_initialization_handler(std::bind(&CAppTest::cb_initialization, &sample_App, _1, _2));
    handler->register_connection_handler(std::bind(&CAppTest::cb_connected, &sample_App, _1, _2));
    handler->register_message_handler(std::bind(&CAppTest::cb_receive_msg_handle, &sample_App, _1, _2));
    handler->register_quit_handler(std::bind(&CAppTest::cb_abnormally_quit, &sample_App, _1));

    handler->init();

    while(true) {
        sleep(1);
    }

    return 0;
}
