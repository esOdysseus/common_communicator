#ifndef C_RAW_MESSAGE_H_
#define C_RAW_MESSAGE_H_

#include <mutex>
#include <memory>

#include <Enum_common.h>

class CRawMessage {
public:
    using MsgDataType = uint8_t;
    using LanAddrType = std::shared_ptr<struct sockaddr_in>;
    using LanSockType = std::shared_ptr<int>;

public:
    CRawMessage(int sockfd=0, size_t capacity=0);

    ~CRawMessage(void);

    void destroy(void);

    int get_socket_fd(void);

    // With Regard to Message
    bool set_new_msg(const void* buf, size_t msize);

    bool append_msg(void* buf, size_t msize);

    size_t get_msg_size(void);

    size_t get_msg(void* buffer, size_t cap);

    const void* get_msg_read_only(size_t* msg_size=NULL);

    // With Regard to Source. (Client Address & Alias name)
    template <typename ADDR_TYPE>
    bool set_source(std::shared_ptr<ADDR_TYPE> addr, const char* alias);

    LanAddrType get_source_addr(std::string& alias, enum_c::ServerType server_type);

    LanSockType get_source_sock(std::string& alias, enum_c::ServerType server_type);

    const struct sockaddr_in* get_source_addr_read_only(enum_c::ServerType server_type);

    std::string get_source_alias(void);

private:
    template <typename ADDR_TYPE>
    enum_c::ServerType policy_addr(void);

    bool extend_capacity(size_t append_capacity);

    bool init(int sockfd, size_t capacity);

    void clear(void);

private:
    class CSource {
    public:
        using EADDR_TYPE = enum_c::ServerType;

    public:
        CSource(void);

        ~CSource(void);

        template <typename ADDR_TYPE> 
        void init(std::shared_ptr<ADDR_TYPE> addr, 
                  const char* alias, 
                  enum_c::ServerType server_type);

        template <typename ADDR_TYPE> 
        std::shared_ptr<ADDR_TYPE> get_address(void);

        std::string get_alias(void);

        EADDR_TYPE get_addr_type(void);

    private:
        std::shared_ptr<void> address;

        std::string alias;

        EADDR_TYPE addr_type;
    };

    static const size_t capacity_bin = 1024U;

    size_t capacity;

    size_t msg_size;

    MsgDataType* msg;

    int socketfd;

    CSource source;

    std::mutex mtx_copy;
};

#endif // C_RAW_MESSAGE_H_
