#ifndef _CPAYLOAD_H_
#define _CPAYLOAD_H_

#include <list>
#include <string>
#include <memory>

#include <CRawMessage.h>

class IProtocolInf;

namespace payload
{
    typedef enum E_ERROR {
        E_NO_ERROR = 0,
        E_ITS_NOT_SUPPORTED_TYPE = 3,
        E_INVALID_VALUE = 4
    }E_ERROR;


    class CPayload : public std::enable_shared_from_this<CPayload> {
    protected:
        using DataType = CRawMessage;

    private:
        using PayloadType = std::shared_ptr<CPayload>;
        using SharedThisType = std::enable_shared_from_this<CPayload>;
        static constexpr char* Default_Name = "none";

    public:
        CPayload(std::string name = Default_Name);

        virtual ~CPayload(void);

        /** Get Protocol-Name */
        const std::string get_name(void);

        /** Get Protocol Instance */
        std::shared_ptr<IProtocolInf> get(std::string proto_name);

        /** Get Payload */
        const void* get_payload(size_t& payload_length);

        /** Set Payload */
        bool set_payload(const void* msg, size_t msg_size);

        /** is payload empty? */
        bool is_empty(void);

    protected:
        /** Get Payload */
        std::shared_ptr<DataType> get_payload(void);

        /** Set Payload */
        bool set_payload(std::shared_ptr<DataType>&& msg_raw);

        void insert_next(PayloadType&& payload);

        PayloadType get_next(void);

    private:
        std::string _name_;

        std::shared_ptr<DataType> _payload_;

        PayloadType next;

        std::shared_ptr<std::list<CPayload>> proto_chain;   // link to external list-object.

    };

}

#endif // _CPAYLOAD_H_