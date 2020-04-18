#ifndef C_SERVER_TCP_H_
#define C_SERVER_TCP_H_

#include <map>
#include <string>
#include <IServerInf.h>
#include <IAppInf.h>
#include <IProtocolInf.h>
#include <CAliasAddr.h>

class CServerTCP : public IServerInf {
public:
    CServerTCP(AliasType& alias_list);

    ~CServerTCP(void);

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
    int make_socket(int opt_flag);

    int get_connected_socket(std::string alias, MessageType &msg);

    std::string make_client_id(const int addr_type, const struct sockaddr_in& cliaddr);

    void make_sockaddr_in(struct sockaddr_in &addr, const char* ip, unsigned int port);

    void server_accept(void);

    int get_self_sockfd(void);              // for only Client-Mode.

    void release_self_sockfd(int sockfd);    // for only Client-Mode.

private:
    ProviderMode _mode_;

    struct sockaddr_in servaddr;    // server-side address

    struct sockaddr_in cliaddr;    // server-side address

    int _available_sockfd_;         // it's only for client-Mode. (Announce now-available sockfd)

    CAliasAddr<struct sockaddr_in, CALIAS_CMPFUNC_for_sockaddr_in> mAddr;   // alias : essential-address

    CAliasAddr<int> m_alias2socket;      // alias => socket

};

#endif // C_SERVER_TCP_H_
