/***************************************************************************
 * 
 * C++ type: Concurrent multiple-Server with each specific Protocol. (IPv4)
 * 
 * *************************************************************************/

#include <iostream>
#include <unistd.h>

#include <ICommunicator.h>
#include <CAppTest.h>

using namespace std::placeholders;

int main(int argc, char *argv[])
{
    std::cout << "Sample-Client for check operating of TCP provider." << std::endl;

    // Create Communicator instance.
    auto handler = std::make_shared<ICommunicator>( "TestApp04", 
                                                    "UnregisteredPVD", 
                                                    argv[1],
                                                    argv[2],
                                                    enum_c::ProviderType::E_PVDT_TRANS_TCP,
                                                    0,
                                                    (const char*)NULL,
                                                    enum_c::ProviderMode::E_PVDM_BOTH);

    std::cout << "Common-API Version = " << handler->get_version() << std::endl;

    // Register Call-Back function pointer of CAppTest class.
    CAppTest sample_App(handler);
    sample_App.set_peer(argv[3], atoi(argv[4]), "Unknown-APP", "Unknown-PVD");
    
    handler->register_initialization_handler(std::bind(&CAppTest::cb_initialization, &sample_App, _1, _2));
    handler->register_connection_handler(std::bind(&CAppTest::cb_connected, &sample_App, _1, _2, _3));
    handler->register_message_handler(std::bind(&CAppTest::cb_receive_msg_handle, &sample_App, _1, _2, _3));
    handler->register_unintended_quit_handler(std::bind(&CAppTest::cb_abnormally_quit, &sample_App, _1));

    handler->init();

    while(true) {
        sleep(1);
    }

    return 0;
}
