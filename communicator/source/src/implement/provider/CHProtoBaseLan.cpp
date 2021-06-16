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
#include <provider/CHProtoBaseLan.h>
#include <IAliasPVD.h>

using namespace std::placeholders;

CHProtoBaseLan::CHProtoBaseLan(std::shared_ptr<IPVDInf> &&provider, AppCallerType& app, std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager)
: IHProtocolInf(provider, app, proto_manager) {
    // this->s_server = std::forward<std::shared_ptr<IPVDInf>>(server);
    assert( set_app_call_back() == true );
}

CHProtoBaseLan::~CHProtoBaseLan(void) {
    // s_server.reset();
}

bool CHProtoBaseLan::set_app_call_back(void) {
    AppCallerType& app = get_app_instance();

    app->set_send_payload_of_app(bind(&CHProtoBaseLan::write_payload, this, _1, _2, _3));
    return true;
}

/************************
 * This API support that setting Specific-property of 3th-party Protocol.
 */ 

/***
 * parameter app_path [in] path about receiver-APP to receive the message.
 * parameter pvd_path [in] provider-path in receiver-APP to receive the message.
 * parameter payload [in] payload to send.
 */
bool CHProtoBaseLan::write_payload(std::string app_path, std::string pvd_path, std::shared_ptr<payload::CPayload>&& payload) {
    LOGD("It's called.");
    ProtocolType pro_payload = std::dynamic_pointer_cast<IProtocolInf>(payload);

    try {
        AppCallerType& app = get_app_instance();
        std::string from_full_path = cf_alias::IAlias::make_full_path( app->get_app_id(), app->get_provider_id() );
        SegmentsType segments = encapsulation(pro_payload, m_provider->get_provider_type(), std::move(from_full_path));

        // message write.
        for(SegmentsType::iterator itor = segments.begin(); itor != segments.end(); itor++) {
            if(m_provider->write_msg(app_path, pvd_path, *itor) != true) {
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
        destroy_proto_chain(pro_payload);
        LOGERR("%s", e.what());
    }
    return false;
}
