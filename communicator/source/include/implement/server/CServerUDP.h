#ifndef C_SERVER_UDP_H
#define C_SERVER_UDP_H

#include <map>
#include <string>
#include <IServerInf.h>
#include <IAppInf.h>
#include <server/CHProtoBaseLan.h>

class CServerUDP : public IServerInf<CHProtoBaseLan> {
public:
    using AddressType = CRawMessage::LanAddrType;
    using AddressMapType = std::map<std::string, AddressType>;

public:
    CServerUDP(void);

    ~CServerUDP(void);

    bool init(std::string id, unsigned int port, const char* ip=NULL) override;

    bool start(void) override;

    bool accept(AppCallerType &app, std::shared_ptr<CConfigProtocols> &proto_manager) override;

    MessageType read_msg(int u_sockfd, bool &is_new) override;

    bool write_msg(std::string client_id, MessageType msg) override;

protected:
    int enable_keepalive(int sock) override;

private:
    bool isthere_addr(std::string alias);
    
    AddressType get_addr(std::string alias);

    bool insert_addr(std::string alias, const struct sockaddr_in &address, bool & is_new);

    void remove_addr(std::string alias);

    void clear_addr(void);

private:
    AddressMapType m_alias_addr;

};

#endif // C_SERVER_UDP_H