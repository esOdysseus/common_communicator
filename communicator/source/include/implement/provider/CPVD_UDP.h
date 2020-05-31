/***
 * CPVD_UDP.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef C_PROVIDER_UDP_H
#define C_PROVIDER_UDP_H

#include <string>
#include <IPVDInf.h>
#include <IAppInf.h>
#include <Cinet_uds.h>

template <typename ADDR_TYPE=struct sockaddr_in>
class CPVD_UDP : public IPVDInf, Cinet_uds {
public:
    CPVD_UDP(AliasType& alias_list);

    ~CPVD_UDP(void);

    bool init(std::string id, uint16_t port=0, const char* ip=NULL, ProviderMode mode=ProviderMode::E_PVDM_BOTH) override;

    bool start(AppCallerType &app, std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager) override;

    bool stop(void) override;

    bool accept(void) override;

    int make_connection(std::string alias) override;

    void disconnection(std::string alias) override;

    MessageType read_msg(int u_sockfd, bool &is_new) override;

    bool write_msg(std::string alias, MessageType msg) override;

protected:
    int enable_keepalive(int sock) override;

    void update_alias_mapper(AliasType& alias_list, std::string &res_alias_name) override;

    bool update_alias_mapper(AliasType& alias_list) override;

    void run_receiver(std::string alias, bool *is_continue) override;

private:
    std::string make_client_id(const ADDR_TYPE& cliaddr);

private:
    ADDR_TYPE servaddr;    // server-side address

    CAliasAddr<ADDR_TYPE, CALIAS_CMPFUNC<ADDR_TYPE>> mAddr;   // alias : essential-address

    bool _is_continue_;

};

#endif // C_PROVIDER_UDP_H
