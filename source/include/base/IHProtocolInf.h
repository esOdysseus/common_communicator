
#ifndef I_HANDLER_PROTOCOL_INTERFACE_H_
#define I_HANDLER_PROTOCOL_INTERFACE_H_

#include <list>
#include <cassert>
#include <string>
#include <memory>
#include <iostream>

#include <IAppInf.h>
#include <CRawMessage.h>
#include <Enum_common.h>
#include <BaseDatatypes.h>
#include <CPayload.h>

class IHProtocolInf {
public:
    using RawMsgType = dtype_b::RawMsgType;
    using MsgType = dtype_b::MsgType;
    using SegmentsType = dtype_b::SegmentsType;
    using AppCallerType = dtype_b::AppCallerType;
    using AppType = dtype_b::AppType;

public:
    template <typename SERVER> 
    IHProtocolInf(std::string client_addr, 
                  int socket_handler, 
                  std::shared_ptr<SERVER> &&server,
                  AppCallerType &app) {
        assert(client_addr.empty()==false);

        this->t_id = client_addr;
        this->h_socket = socket_handler;
        this->s_app = app;
        this->client_id = client_addr;
        set_running_flag(false);
    }

    ~IHProtocolInf(void);

    virtual void run(void) = 0;
    
    virtual bool destroy(void) = 0;

    bool get_running_flag(void);

    std::string get_thread_id(void);

    std::string get_client_id(void);

protected:
    void set_running_flag(bool value);

    virtual bool set_app_call_back(void) = 0;

    virtual bool write(std::string client_id, const void* msg, size_t msg_size) = 0;

    virtual bool write_payload(std::string client_id, std::shared_ptr<CPayload>&& payload) = 0;

    template <typename PROTOCOL> 
    SegmentsType encapsulation(const void* msg_raw, size_t msg_size, enum_c::ServerType server_type) {
        /****
         * According to ServerType, fragment the message. & make segment-List.
         *                        - segment-list will be RawMsgType-List.
         * Segments : 1. combine with One-Head + payload + (One-Tail)
         *          : 2. Encoding(payload)
         */
        std::shared_ptr<PROTOCOL> p_msg = std::make_shared<PROTOCOL>();
        try{
            return p_msg->pack_recursive(msg_raw, msg_size, server_type);
        }catch(const std::exception &e) {
            std::cout << "[Error] IHProtocolInf::encapsulation() : " << e.what() << std::endl;
            throw std::exception(e);
        }
    }

    template <typename PROTOCOL> 
    SegmentsType encapsulation(std::shared_ptr<PROTOCOL>& p_msg, enum_c::ServerType server_type) {
        /****
         * According to ServerType, fragment the message. & make segment-List.
         *                        - segment-list will be RawMsgType-List.
         * Segments : 1. combine with One-Head + payload + (One-Tail)
         *          : 2. Encoding(payload)
         */
        // std::shared_ptr<PROTOCOL> p_msg = std::make_shared<PROTOCOL>();
        size_t msg_size = 0;
        const void* msg_raw = p_msg->get_payload()->get_msg_read_only(&msg_size);

        try{
            return p_msg->pack_recursive(msg_raw, msg_size, server_type);
        }catch(const std::exception &e) {
            std::cout << "[Error] IHProtocolInf::encapsulation() : " << e.what() << std::endl;
            throw std::exception(e);
        }
    }

    template <typename PROTOCOL> 
    std::shared_ptr<PROTOCOL> decapsulation(RawMsgType msg_raw) {
        /****
         * Classify segments from the raw-message. & extract payload & It combine with payloads.
         * Segments : 1. combine with One-Head + payload + (One-Tail)
         *          : 2. Decoding(payload)
         */
        bool res = false;
        std::shared_ptr<PROTOCOL> p_msg = std::make_shared<PROTOCOL>();
        try{
            res = p_msg->unpack_recurcive(msg_raw->get_msg_read_only(), msg_raw->get_msg_size());
            assert(res==true);
        }
        catch(const std::exception &e) {
            std::cout << "[Error] IHProtocolInf::decapsulation() : " << e.what() << std::endl;
            throw std::exception(e);
        }
        return p_msg;
    }

    AppCallerType& get_app_instance(void);

    int get_sockfd(void);

private:
    AppCallerType s_app;
    
    int h_socket;

    std::string t_id;

    std::string client_id;

    bool f_is_run;
};

#endif // I_HANDLER_PROTOCOL_INTERFACE_H_