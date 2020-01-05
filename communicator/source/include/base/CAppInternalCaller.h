#ifndef _CAPP_INTERNAL_CALLER_H_
#define _CAPP_INTERNAL_CALLER_H_

#include <string>
#include <functional>

#include <CReceiver.h>

class CAppInternalCaller {
public:
    using SendToType = std::function<bool(std::string client_id, const void* msg, size_t msg_size)>;
    using SendPayloadType = std::function<bool(std::string client_id, std::shared_ptr<payload::CPayload>&& payload)>;

    using AppSendTo_SetterType = std::function<void (SendToType fp)>;
    using AppSendPayload_SetterType = std::function<void (SendPayloadType fp)>;
    using AppCBhandler_GetterType = std::function<CReceiver& (void)>;
    using AppID_GetterType = std::function<std::string (void)>;

public:
    AppSendTo_SetterType        set_sendto_of_app;
    AppSendPayload_SetterType   set_send_payload_of_app;
    AppCBhandler_GetterType     get_cb_handlers;
    AppID_GetterType            get_app_id;

public:
    CAppInternalCaller(void) { clear(); }

    ~CAppInternalCaller(void) { clear(); }

private:
    void clear(void) {
        set_sendto_of_app = NULL;
        set_send_payload_of_app = NULL;
        get_cb_handlers = NULL;
        get_app_id = NULL;
    }

};

#endif // _CAPP_INTERNAL_CALLER_H_