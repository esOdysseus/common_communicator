#ifndef _CPAYLOAD_H_
#define _CPAYLOAD_H_

#include <string>
#include <memory>


class IProtocolInf;

namespace payload
{
    typedef enum E_ERROR {
        E_NO_ERROR = 0,
        E_ITS_NOT_SUPPORTED_TYPE = 3,
        E_INVALID_VALUE = 4
    }E_ERROR;


    class CPayload : public std::enable_shared_from_this<CPayload> {
    public:
        CPayload(std::string name);

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

    };

}

#endif // _CPAYLOAD_H_