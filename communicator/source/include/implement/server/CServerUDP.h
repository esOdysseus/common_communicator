#ifndef C_SERVER_UDP_H
#define C_SERVER_UDP_H

#include <string>
#include <IServerInf.h>
#include <IAppInf.h>
#include <server/CHProtoBaseLan.h>

class CServerUDP : public IServerInf<CHProtoBaseLan> {

public:
    CServerUDP(AliasType& alias_list);

    ~CServerUDP(void);

    bool init(std::string id, unsigned int port=0, const char* ip=NULL) override;

    bool start(AppCallerType &app, std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager) override;

    bool accept(void) override;

    int make_connection(std::string alias) override;

    bool disconnection(std::string alias) override;

    MessageType read_msg(int u_sockfd, bool &is_new) override;

    bool write_msg(std::string alias, MessageType msg) override;

protected:
    int enable_keepalive(int sock) override;

    bool update_alias_mapper(AliasType& alias_list) override;

    void run_receiver(std::string alias, bool *is_continue) override;

private:
    bool _is_continue_;

};

#endif // C_SERVER_UDP_H