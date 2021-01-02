/***
 * CPVD_TCP.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef C_PROVIDER_TCP_H_
#define C_PROVIDER_TCP_H_

#include <map>
#include <string>
#include <IPVDInf.h>
#include <IAppInf.h>
#include <IProtocolInf.h>
#include <CAliasAddr.h>
#include <Cinet_uds.h>

template <typename ADDR_TYPE=struct sockaddr_in>
class CPVD_TCP : public IPVDInf, Cinet_uds {
public:
    CPVD_TCP(AliasPVDsType& alias_list);

    ~CPVD_TCP(void);

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

    void update_alias_mapper(AliasPVDsType& alias_list, std::string &res_alias_name) override;

    bool update_alias_mapper(AliasPVDsType& alias_list) override;

    void run_receiver(std::string alias, bool *is_continue) override;

private:
    int make_socket(int opt_flag);

    int get_connected_socket(std::string alias, MessageType &msg);

    std::string make_client_id(const ADDR_TYPE& cliaddr);

    void server_accept(void);

    int get_self_sockfd(void);              // for only Client-Mode.

    void release_self_sockfd(int sockfd);    // for only Client-Mode.

private:
    ProviderMode _mode_;

    ADDR_TYPE servaddr;    // server-side address

    ADDR_TYPE cliaddr;    // server-side address

    int _available_sockfd_;         // it's only for client-Mode. (Announce now-available sockfd)

    CAliasAddr<ADDR_TYPE, CALIAS_CMPFUNC<ADDR_TYPE>> mAddr;   // alias : essential-address

    CAliasAddr<int> m_alias2socket;      // alias => socket

};

#endif // C_PROVIDER_TCP_H_
