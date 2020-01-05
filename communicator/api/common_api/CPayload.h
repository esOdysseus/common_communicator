#ifndef _CPAYLOAD_H_
#define _CPAYLOAD_H_

#include <string>
#include <memory>

#include <CRawMessage.h>


namespace payload
{
    typedef enum E_ERROR {
        E_NO_ERROR = 0,
        E_ITS_NOT_SUPPORTED_TYPE = 3,
        E_INVALID_VALUE = 4
    }E_ERROR;

    #define GET_PROTOCOL(PROTOCOL, payload) std::dynamic_pointer_cast<IProtocolInf>( (payload)->get(PROTOCOL) )

    class CPayload : public std::enable_shared_from_this<CPayload> {
    public:
        using DataType = CRawMessage;
        using PayloadType = std::shared_ptr<CPayload>;

    private:
        using SharedThisType = std::enable_shared_from_this<CPayload>;
        static constexpr char* Default_Name = "none";

    public:
        CPayload(std::string name = Default_Name);

        virtual ~CPayload(void);

        std::shared_ptr<CPayload> get(std::string proto_name);

        const std::string get_name(void);

        std::shared_ptr<DataType> get_payload(void);

        const void* get_payload(size_t& payload_length);

        bool set_payload(const void* msg, size_t msg_size);

        bool set_payload(std::shared_ptr<DataType>&& msg_raw);

        bool is_there_data(void);

        void insert_next(PayloadType&& payload);

    protected:
        PayloadType get_next(void);

    private:
        std::string _name_;

        std::shared_ptr<DataType> _payload_;

        PayloadType next;

    };

}

#endif // _CPAYLOAD_H_