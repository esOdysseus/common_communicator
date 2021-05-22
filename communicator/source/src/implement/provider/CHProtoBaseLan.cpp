/***
 * CHProtoBaseLan.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <cassert>
#include <logger.h>
#include <Enum_common.h>
#include <IAppInf.h>
#include <provider/CHProtoBaseLan.h>

using namespace std::placeholders;

CHProtoBaseLan::CHProtoBaseLan(ServerType &&server, AppCallerType& app, std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager)
: IHProtocolInf(app, proto_manager) {
    this->s_server = std::forward<ServerType>(server);
    assert( set_app_call_back() == true );
}

CHProtoBaseLan::~CHProtoBaseLan(void) {
    s_server.reset();
}

bool CHProtoBaseLan::set_app_call_back(void) {
    AppCallerType& app = get_app_instance();

    app->set_send_payload_of_app(bind(&CHProtoBaseLan::write_payload, this, _1, _2));
    return true;
}

/************************
 * This API support that setting Specific-property of 3th-party Protocol.
 */ 

/***
 * parameter alias [in] receiver-alias to receive the message.
 * parameter payload [in] payload to send.
 */
bool CHProtoBaseLan::write_payload(std::string alias, std::shared_ptr<payload::CPayload>&& payload) {
    LOGD("It's called.");

    try {
        AppCallerType& app = get_app_instance();
        ProtocolType pro_payload = std::dynamic_pointer_cast<IProtocolInf>(payload);
        SegmentsType segments = encapsulation(pro_payload, s_server->get_provider_type(), 
                                              app->get_provider_id());

        // message write.
        for(SegmentsType::iterator itor = segments.begin(); itor != segments.end(); itor++) {
            if(s_server->write_msg(alias, *itor) != true) {
                throw std::logic_error("write message is failed.");
            }
        }

        if ( payload->get_op_flag(payload::E_PAYLOAD_FLAG::E_KEEP_PAYLOAD_AFTER_TX) == false ) {
            destroy_proto_chain(pro_payload);
        }
        else {
            LOGW("E_KEEP_PAYLOAD_AFTER_TX == true");
            pro_payload->clean_data(true, false);
        }

        return true;
    }
    catch(const std::exception &e) {
        LOGERR("%s", e.what());
    }
    return false;
}
