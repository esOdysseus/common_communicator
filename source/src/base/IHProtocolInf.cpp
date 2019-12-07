
#include <unistd.h>
#include <cassert>
#include <IHProtocolInf.h>

IHProtocolInf::~IHProtocolInf(void) {
    if (h_socket != 0) {
        close(h_socket);
    }
    t_id = "destroyed";
    h_socket = 0;
    set_running_flag(false);
    s_app.reset();
    client_id = "";
}

bool IHProtocolInf::get_running_flag(void) {
    return f_is_run;
}

std::string IHProtocolInf::get_thread_id(void) {
    return t_id;
}

std::string IHProtocolInf::get_client_id(void) {
    return client_id;
}

void IHProtocolInf::set_running_flag(bool value) {
    f_is_run = value;
}

IHProtocolInf::AppCallerType& IHProtocolInf::get_app_instance(void) {
    return s_app;
}

int IHProtocolInf::get_sockfd(void) {
    return h_socket;
}
