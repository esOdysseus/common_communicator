/***
 * CServerUDP.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef C_SERVER_UDP_H
#define C_SERVER_UDP_H

#include <string>
#include <IServerInf.h>
#include <IAppInf.h>

class CServerUDP : public IServerInf {
public:
    CServerUDP(AliasType& alias_list);

    ~CServerUDP(void);

    bool init(std::string id, unsigned int port=0, const char* ip=NULL, ProviderMode mode=ProviderMode::E_PVDM_BOTH) override;

    bool start(AppCallerType &app, std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager) override;

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
    std::string make_client_id(const int addr_type, const struct sockaddr_in& cliaddr);

private:
    struct sockaddr_in servaddr;    // server-side address

    CAliasAddr<struct sockaddr_in, CALIAS_CMPFUNC_for_sockaddr_in> mAddr;   // alias : essential-address

    bool _is_continue_;

};

#endif // C_SERVER_UDP_H
