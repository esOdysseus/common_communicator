/***
 * IServerInf.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef ISERVER_INTERFACE_H_
#define ISERVER_INTERFACE_H_

#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <netinet/in.h>

#include <IHProtocolInf.h>
#include <CRawMessage.h>
#include <Enum_common.h>
#include <BaseDatatypes.h>
#include <CConfigProtocols.h>
#include <CAliasAddr.h>
#include <CAliasCompare.h>
#include <CConfigAliases.h>

class IHProtocolInf;

class IServerInf: public std::enable_shared_from_this<IServerInf> {
public:
    using AppCallerType = dtype_b::AppCallerType;
    using HProtocolType = std::shared_ptr<IHProtocolInf>;
    using SharedThisType = std::enable_shared_from_this<IServerInf>;
    using ThreadType = std::shared_ptr<std::thread>;
    using MessageType = dtype_b::MsgType;
    using AliasType = std::list<std::shared_ptr<cf_alias::IAlias>>;
    using FPreceiverType = std::function<void(std::string alias, bool *is_continue)>;

protected:
    using ProviderMode = enum_c::ProviderMode;

private:
    class CLooper;

    using LoopPoolType = std::unordered_map<std::string /* alias */, std::shared_ptr<CLooper> >;

public:
    IServerInf(AliasType& alias_list);

    ~IServerInf(void);

    virtual bool init(std::string id, unsigned int port=0, const char* ip=NULL, ProviderMode mode=ProviderMode::E_PVDM_BOTH) = 0;

    virtual bool start(AppCallerType &app, std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager) = 0;

    virtual bool accept(void) = 0;

    virtual int make_connection(std::string alias) = 0;

    virtual void disconnection(std::string alias) = 0;

    virtual MessageType read_msg(int u_sockfd, bool &is_new) = 0;

    virtual bool write_msg(std::string alias, MessageType msg) = 0;

    bool register_new_alias(const char* peer_ip, uint16_t peer_port, std::string &wanted_name);

    bool stop(void);

    std::string get_id(void) { return id; }

    enum_c::ProviderType get_provider_type(void) { return provider_type; }

    int gen_random_portnum(void);

protected:
    virtual int enable_keepalive(int sock) = 0;

    // Temporary Function : alias 전달후 issue 발생으로 임시적으로 기능 구현을 위해 존재함.
    virtual void update_alias_mapper(AliasType& alias_list, std::string &res_alias_name) = 0;

    virtual bool update_alias_mapper(AliasType& alias_list) = 0;

    virtual void run_receiver(std::string alias, bool *is_continue) = 0;

    void clear(void);

    template <typename PROTOCOL_H> 
    bool create_hprotocol(AppCallerType& app, std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager);

    bool thread_create(std::string& client_id, FPreceiverType &&func);

    bool thread_this_migrate(std::string& client_id, FPreceiverType &&func, bool *is_continue);

    bool thread_destroy(std::string client_id);

    void set_id(std::string& value) { id = value; }

    void set_provider_type(enum_c::ProviderType type) { provider_type = type; }

protected:
    bool started;

    bool inited;

    int sockfd;   // start server to listen for clients to send them ids

    unsigned int listeningPort;

    std::mutex mtx_write, mtx_read;

    HProtocolType hHprotocol;   // handle of Protocol-Handler

    static const unsigned int read_bufsize = 2048;

    static const unsigned short peer_name_bufsize = 20;

    char read_buf[read_bufsize];

private:
    std::string id;

    LoopPoolType mLooperPool;

    enum_c::ProviderType provider_type;

};

#endif // ISERVER_INTERFACE_H_
