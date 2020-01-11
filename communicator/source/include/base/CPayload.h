#ifndef _CPAYLOAD_H_
#define _CPAYLOAD_H_

#include <list>
#include <string>
#include <memory>

#include <CRawMessage.h>


class IProtocolInf;
namespace cf_proto {
    class CConfigProtocols;
}

namespace payload
{
    typedef enum E_ERROR {
        E_NO_ERROR = 0,
        E_INVALID_MEMBER_VARIABLES = 2,
        E_ITS_NOT_SUPPORTED_TYPE = 3,
        E_INVALID_VALUE = 4
    }E_ERROR;


    class CPayload : public std::enable_shared_from_this<CPayload> {
    public:
        static constexpr char* Myself_Name = "_myself_";

    protected:
        using DataType = CRawMessage;
        using PayloadType = std::shared_ptr<CPayload>;
        using ProtoChainType = std::list<PayloadType>;
        static constexpr char* Default_Name = "none";

    private:
        using SharedThisType = std::enable_shared_from_this<CPayload>;

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
        /** Get Protocol that is contained by myself-payload. */
        std::shared_ptr<IProtocolInf> get_protocol(void);

        /** Get Payload */
        std::shared_ptr<DataType> get_payload(void);

        /** Set Payload */
        bool set_payload(std::shared_ptr<DataType>&& msg_raw);

        /** Get Protocol-Chain instance. */
        std::shared_ptr<ProtoChainType> get_proto_chain(void);

        /** Set Protocol-Chain instance. */
        void set_proto_chain(std::string chain_name, std::shared_ptr<ProtoChainType>& proto_chain);

        /** Get Protocol-Chain name. */
        std::string get_protocols_chain_name(void);

        friend class cf_proto::CConfigProtocols;

    private:
        std::string _name_;

        std::shared_ptr<DataType> _payload_;

        std::string _protocol_chain_name_;      // link to external list-object.

        std::weak_ptr<ProtoChainType> _protocol_chain_;   // link to external list-object.

    };

}

#endif // _CPAYLOAD_H_
