#ifndef C_SERVER_TCP_H_
#define C_SERVER_TCP_H_

#include <map>
#include <string>
#include <IServerInf.h>
#include <IAppInf.h>
#include <server/CHProtoBaseLan.h>
#include <IProtocolInf.h>
#include <CAliasAddr.h>

class CServerTCP : public IServerInf<CHProtoBaseLan> {
public:
    CServerTCP(AliasType& alias_list);

    ~CServerTCP(void);

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
    int make_socket(int opt_flag);

    int get_socket(std::string alias, MessageType &msg);

private:
    CAliasAddr<int> m_alias2socket;      // alias => socket

};

#endif // C_SERVER_TCP_H_