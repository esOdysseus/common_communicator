#ifndef C_SERVER_TCP_H_
#define C_SERVER_TCP_H_

#include <map>
#include <string>
#include <IServerInf.h>
#include <IAppInf.h>
#include <server/CHProtoBaseLan.h>
#include <IProtocolInf.h>

class CServerTCP : public IServerInf<CHProtoBaseLan<CNoneProtocol>> {
public:
    using SocketMapType = std::map<int, std::string>;  // socket, client-id

public:
    CServerTCP(void);

    ~CServerTCP(void);

    bool init(std::string id, unsigned int port, const char* ip=NULL) override;

    bool start(void) override;

    bool accept(AppCallerType &app) override;

    MessageType read_msg(int u_sockfd) override;

    bool write_msg(std::string client_id, MessageType msg) override;

protected:
    int enable_keepalive(int sock) override;

private:
    bool isthere_client_id(const int socket_num);

    std::string get_client_id(const int socket_num);

    bool insert_client(const int socket_num, std::string alias);

    void remove_client(const int socket_num);

    void clear_clients(void);

private:
    SocketMapType m_socket_alias;

};

#endif // C_SERVER_TCP_H_