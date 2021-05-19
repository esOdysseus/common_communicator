/***
 * CAppInternalCaller.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef _CAPP_INTERNAL_CALLER_H_
#define _CAPP_INTERNAL_CALLER_H_

#include <string>
#include <functional>

#include <CReceiver.h>

class CAppInternalCaller {
public:
    using SendPayloadType = std::function<bool(std::string client_id, std::shared_ptr<payload::CPayload>&& payload)>;

    using AppSendPayload_SetterType = std::function<void (SendPayloadType fp)>;
    using AppCBhandler_GetterType = std::function<CReceiver& (void)>;
    using AppID_GetterType = std::function<std::string (void)>;

public:
    AppSendPayload_SetterType   set_send_payload_of_app;
    AppCBhandler_GetterType     get_cb_handlers;
    AppID_GetterType            get_app_id;
    AppID_GetterType            get_provider_id;

public:
    CAppInternalCaller(void) { clear(); }

    ~CAppInternalCaller(void) { clear(); }

private:
    void clear(void) {
        set_send_payload_of_app = NULL;
        get_cb_handlers = NULL;
        get_app_id = NULL;
        get_provider_id = NULL;
    }

};

#endif // _CAPP_INTERNAL_CALLER_H_