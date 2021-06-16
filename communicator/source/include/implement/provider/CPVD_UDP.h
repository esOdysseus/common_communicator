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
#include <Cinet_uds.h>

template <typename ADDR_TYPE=struct sockaddr_in>
class CPVD_UDP : public IPVDInf, Cinet_uds {
public:
    CPVD_UDP(std::shared_ptr<cf_alias::IAliasPVD> self_alias, std::shared_ptr<cf_alias::CConfigAliases>& alia_manager, AliasPVDsType& alias_list);

    ~CPVD_UDP(void);

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

    void update_provider(std::shared_ptr<cf_alias::IAliasPVD> peer_alias, std::string &ip, uint16_t &port );

private:
    std::shared_ptr<cf_alias::IAliasPVD> make_new_client_alias(std::string pvd_id, const char* ip, const uint16_t port);

    std::shared_ptr<cf_alias::IAliasPVD> make_client_id(const ADDR_TYPE& cliaddr);

private:
    ADDR_TYPE servaddr;    // server-side address

    CAliasAddr<ADDR_TYPE, CALIAS_CMPFUNC<ADDR_TYPE>> _mm_ali4addr_;   // alias : essential-address

    bool _is_continue_;

};

#endif // C_PROVIDER_UDP_H
