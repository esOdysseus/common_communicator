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
    std::cout << "Sample-Client for check operating of UDS_TCP provider." << std::endl;

    try {
        // Create Communicator instance.
        auto handler = std::make_shared<ICommunicator>( "TestApp04", 
                                                        "UnregistedPVD", 
                                                        argv[1],
                                                        argv[2],
                                                        enum_c::ProviderType::E_PVDT_TRANS_UDS_TCP,
                                                        0,
                                                        (const char*)NULL,
                                                        enum_c::ProviderMode::E_PVDM_BOTH);
        if ( handler.get() == NULL ) {
            throw std::runtime_error("Can not allocate memoty to ICommunicator class.");
        }

        std::cout << "Common-API Version = " << handler->get_version() << std::endl;

        // Register Call-Back function pointer of CAppTest class.
        CAppTest sample_App(handler);
        handler->register_initialization_handler(std::bind(&CAppTest::cb_initialization, &sample_App, _1, _2));
        handler->register_connection_handler(std::bind(&CAppTest::cb_connected, &sample_App, _1, _2, _3, _4, _5));
        handler->register_message_handler(std::bind(&CAppTest::cb_receive_msg_handle, &sample_App, _1, _2, _3));
        handler->register_unintended_quit_handler(std::bind(&CAppTest::cb_abnormally_quit, &sample_App, _1));

        handler->init();

        while(true) {
            sleep(1);
        }
    }
    catch ( const std::exception & e ) {
        std::cout << "Error: " << e.what() << std::endl;
        std::cout << "Program End." << std::endl;
    }

    return 0;
}
