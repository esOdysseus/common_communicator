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
#include <IProtocolInf.h>
#include <CAliasAddr.h>
#include <IAliasPVD.h>
#include <Cinet_uds.h>

template <typename ADDR_TYPE=struct sockaddr_in>
class CPVD_TCP : public IPVDInf, Cinet_uds {
public:
    CPVD_TCP(std::shared_ptr<cf_alias::IAliasPVD> self_alias, AliasPVDsType& alias_list);

    ~CPVD_TCP(void);

    bool init(uint16_t port=0, std::string ip=std::string(), ProviderMode mode=ProviderMode::E_PVDM_BOTH) override;

    bool start(AppCallerType &app, std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager) override;

    bool stop(void) override;

    bool accept(void) override;

    int make_connection(std::string peer_full_path) override;

    void disconnection(std::string app_path, std::string pvd_id) override;

    MessageType read_msg(int u_sockfd, bool &is_new) override;

    bool write_msg(std::string app_path, std::string pvd_path, MessageType msg) override;

protected:
    int enable_keepalive(int sock) override;

    void update_alias_mapper(std::shared_ptr<cf_alias::IAliasPVD> new_pvd) override;

    bool update_alias_mapper(AliasPVDsType& alias_list) override;

    void run_receiver(std::shared_ptr<cf_alias::IAliasPVD> peer_alias, bool *is_continue) override;

    void disconnection(std::shared_ptr<cf_alias::IAliasPVD> peer_alias) override;

private:
    int make_socket(int opt_flag);

    int get_connected_socket(std::string peer_full_path, MessageType &msg);

    std::shared_ptr<cf_alias::IAliasPVD> make_new_client_alias(std::string pvd_id, const char* ip, const uint16_t port);

    std::shared_ptr<cf_alias::IAliasPVD> make_client_id(const ADDR_TYPE& cliaddr);

    void server_accept(void);

    int get_self_sockfd(void);              // for only Client-Mode.

    void release_self_sockfd(int sockfd);    // for only Client-Mode.

    void update_provider(std::shared_ptr<cf_alias::IAliasPVD> peer_alias, std::string &ip, uint16_t &port );

private:
    ProviderMode _mode_;

    ADDR_TYPE servaddr;    // server-side address

    ADDR_TYPE cliaddr;    // server-side address

    int _available_sockfd_;         // it's only for client-Mode. (Announce now-available sockfd)

    CAliasAddr<ADDR_TYPE, CALIAS_CMPFUNC<ADDR_TYPE>> _mm_ali4addr_;   // alias : essential-address

    CAliasAddr<int> _mm_ali4sock_;      // alias => socket

};

#endif // C_PROVIDER_TCP_H_
