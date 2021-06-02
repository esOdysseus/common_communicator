/***
 * IPVDInf.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef _INTERFACE_PROVIDER_H_
#define _INTERFACE_PROVIDER_H_

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
#include <IAliasPVD.h>

class IHProtocolInf;

class IPVDInf: public std::enable_shared_from_this<IPVDInf> {
public:
    using AppCallerType = dtype_b::AppCallerType;
    using HProtocolType = std::shared_ptr<IHProtocolInf>;
    using SharedThisType = std::enable_shared_from_this<IPVDInf>;
    using ThreadType = std::shared_ptr<std::thread>;
    using MessageType = dtype_b::MsgType;
    using AliasPVDsType = cf_alias::CConfigAliases::PVDListType;
    using FPreceiverType = std::function<void(std::shared_ptr<cf_alias::IAliasPVD> pvd_alias, bool *is_continue)>;

protected:
    using ProviderMode = enum_c::ProviderMode;

private:
    class CLooper;

    using LoopPoolType = std::unordered_map<std::string /* peer_full_path */, std::shared_ptr<CLooper> >;

public:
    IPVDInf(std::shared_ptr<cf_alias::IAliasPVD>& pvd_alias);

    ~IPVDInf(void);

    virtual bool init(uint16_t port=0, std::string ip=std::string(), ProviderMode mode=ProviderMode::E_PVDM_BOTH) = 0;

    virtual bool start(AppCallerType &app, std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager) = 0;

    virtual bool stop(void) = 0;

    virtual bool accept(void) = 0;

    virtual int make_connection(std::string peer_full_path) = 0;

    virtual void disconnection(std::string app_path, std::string pvd_id) = 0;

    virtual MessageType read_msg(int u_sockfd, bool &is_new) = 0;

    virtual bool write_msg(std::string app_path, std::string pvd_path, MessageType msg) = 0;

    bool register_new_alias(const char* peer_ip, uint16_t peer_port, std::string& app_path, std::string &pvd_id);

    bool quit(void);

    enum_c::ProviderType get_provider_type(void);

    std::shared_ptr<cf_alias::IAliasPVD> get_pvd_alias( void );

protected:
    virtual int enable_keepalive(int sock) = 0;

    // Temporary Function : alias 전달후 issue 발생으로 임시적으로 기능 구현을 위해 존재함.
    virtual void update_alias_mapper(std::shared_ptr<cf_alias::IAliasPVD> new_pvd) = 0;

    virtual bool update_alias_mapper(AliasPVDsType& alias_list) = 0;

    virtual void run_receiver(std::shared_ptr<cf_alias::IAliasPVD> peer_alias, bool *is_continue) = 0;

    virtual void disconnection(std::shared_ptr<cf_alias::IAliasPVD> peer_alias) = 0;

    void clear(void);

    template <typename PROTOCOL_H> 
    bool create_hprotocol(AppCallerType& app, std::shared_ptr<cf_proto::CConfigProtocols> &proto_manager);

    bool thread_create(std::shared_ptr<cf_alias::IAliasPVD>& peer_alias, FPreceiverType &&func);

    bool thread_this_migrate(std::shared_ptr<cf_alias::IAliasPVD>& peer_alias, FPreceiverType &&func, bool *is_continue);

    bool thread_destroy(std::string peer_full_path);

    bool zombi_thread_migrate(std::shared_ptr<cf_alias::IAliasPVD> peer_alias);

protected:
    bool started;

    bool inited;

    int sockfd;   // start server to listen for clients to send them ids

    unsigned int listeningPort;

    std::mutex mtx_write, mtx_read;

    std::mutex mtx_looperpool, mtx_loopergarbage;

    HProtocolType hHprotocol;   // handle of Protocol-Handler

    static const unsigned int read_bufsize = 2048;

    static const unsigned short peer_name_bufsize = 20;

    char read_buf[read_bufsize];

private:
    std::shared_ptr<cf_alias::IAliasPVD>  _m_pvd_alias_;

    LoopPoolType mLooperPool;

    LoopPoolType mLooperGarbage;

};

#endif // _INTERFACE_PROVIDER_H_
