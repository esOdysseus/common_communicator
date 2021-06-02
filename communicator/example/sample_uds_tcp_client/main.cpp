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
    std::cout << "Sample-Client for check operating of UDS_TCP provider." << std::endl;

    // Create Communicator instance.
    auto handler = create_communicator("TestApp04", 
                                       "UDS_TCP_provider", 
                                       enum_c::ProviderType::E_PVDT_TRANS_UDS_TCP,
                                       0,
                                       NULL,
                                       enum_c::ProviderMode::E_PVDM_BOTH,
                                       argv[1],
                                       argv[2]);

    std::cout << "Common-API Version = " << handler->get_version() << std::endl;

    // Register Call-Back function pointer of CAppTest class.
    CAppTest sample_App(handler);
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
