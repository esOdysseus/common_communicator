
#ifndef ISERVER_INTERFACE_H_
#define ISERVER_INTERFACE_H_

#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <netinet/in.h>

#include <IAppInf.h>
#include <IHProtocolInf.h>
#include <CRawMessage.h>
#include <Enum_common.h>
#include <BaseDatatypes.h>
#include <json_manipulator.h>

class IHProtocolInf;

template <typename PROTOCOL_H> 
class IServerInf: public std::enable_shared_from_this<IServerInf<PROTOCOL_H>> {
public:
    using AppType = dtype_b::AppType;
    using AppCallerType = dtype_b::AppCallerType;
    using HProtocolType = std::shared_ptr<IHProtocolInf>;
    using SharedThisType = std::enable_shared_from_this<IServerInf<PROTOCOL_H>>;
    using ThreadType = std::shared_ptr<std::thread>;
    using MessageType = dtype_b::MsgType;

public:
    IServerInf(void);

    ~IServerInf(void);

    virtual bool init(std::string id, unsigned int port, const char* ip=NULL) = 0;

    virtual bool start(void) = 0;

    virtual bool accept(AppCallerType &app, Json_DataType &json_manager) = 0;

    virtual MessageType read_msg(int u_sockfd, bool &is_new) = 0;

    virtual bool write_msg(std::string client_id, MessageType msg) = 0;

    bool stop(void);

    std::string get_id(void) { return id; }

    enum_c::ServerType get_server_type(void) { return server_type; }

protected:
    virtual int enable_keepalive(int sock) = 0;

    void clear(void);

    bool thread_create(std::string& client_addr, int socketfd, AppCallerType& app, Json_DataType &json_manager);

    bool thread_this_migrate(std::string& client_addr, int socketfd, AppCallerType& app, Json_DataType &json_manager);

    void set_id(std::string& value) { id = value; }

    void set_server_type(enum_c::ServerType type) { server_type = type; }

    virtual std::string make_client_id(const int addr_type, const struct sockaddr_in& cliaddr);

protected:
    class CLooper;

    bool started;

    bool inited;

    int sockfd;   //start server to listen for clients to send them ids

    struct sockaddr_in servaddr;

    unsigned int listeningPort;

    std::mutex mtx_write, mtx_read;

    std::unordered_map<std::string /* ClientName */, std::shared_ptr<CLooper> > mLooperPool;

    static const unsigned int read_bufsize = 2048;

    char read_buf[read_bufsize];

private:
    std::string id;

    enum_c::ServerType server_type;
    
};

#endif // ISERVER_INTERFACE_H_