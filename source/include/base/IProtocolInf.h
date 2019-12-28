#ifndef INTERFACE_PROTOCOL_H_
#define INTERFACE_PROTOCOL_H_

#include <iostream>
#include <list>
#include <memory>
#include <cassert>

#include <CRawMessage.h>
#include <CPayload.h>
#include <Enum_common.h>
#include <BaseDatatypes.h>

class CNoneProtocol;

// template <typename NEXT_PROTOCOL>
class IProtocolInf : public CPayload {
public:
    using RawMsgType = dtype_b::RawMsgType;
    using MsgType = std::shared_ptr<CPayload::DataType>;
    using SegmentType = dtype_b::SegmentType;
    using SegmentsType = dtype_b::SegmentsType;

public:
    IProtocolInf(void): CPayload() {
        segments.clear();
    }

    IProtocolInf(std::string name): CPayload(name) {
        segments.clear();
    };

    ~IProtocolInf(void) {};

    SegmentsType& pack_recursive(const void* msg, size_t msg_size, enum_c::ServerType server_type) {
        bool res = false;
        
        try {
            // if (std::is_same<NEXT_PROTOCOL, CNoneProtocol>::value == false) {
            //     assert( this->get_next().get() != NULL );

            //     // Get next-protocol from Linked-list of This-protocol.
            //     std::shared_ptr<NEXT_PROTOCOL> next_protocol = std::dynamic_pointer_cast<NEXT_PROTOCOL>(this->get_next());
            //     // Recursive packing processing is start.
            //     SegmentsType& u_segments = next_protocol->pack_recursive(msg, msg_size, server_type);
                
            //     // Current-protocol packing processing is start.
            //     SegmentsType::iterator itor;
            //     for( itor=u_segments.begin(); itor != u_segments.end(); itor++ ) {
            //         RawMsgType& segment = *itor;
            //         res = pack(segment->get_msg_read_only(), segment->get_msg_size(), server_type);
            //         assert(res == true);
            //     }
            // }
            // else {
            //     assert( (res = pack(msg, msg_size, server_type)) == true );
            // }
        }
        catch(const std::exception &e) {
            std::cout << "[Error] IProtocolInf::pack_recursive() : " << e.what() << std::endl;
            get_segments().clear();
        }

        return get_segments();
    }

    bool unpack_recurcive(const void* msg_raw, size_t msg_size) {
        bool res = false;

        try{
            // assert( (res = unpack(msg_raw, msg_size)) == true );
            // assert( (res = is_there_data()) == true );

            // MsgType payload = get_payload();
            // if (std::is_same<NEXT_PROTOCOL, CNoneProtocol>::value == false) {
            //     std::shared_ptr<NEXT_PROTOCOL> next_protocol = std::make_shared<NEXT_PROTOCOL>();
            //     res = next_protocol->unpack_recurcive(payload->get_msg_read_only(), 
            //                                           payload->get_msg_size());
            //     assert(res == true);
            //     payload.reset();
            //     payload = move(next_protocol->get_payload());
            // }
        }
        catch(const std::exception &e) {
            std::cout << "[Error] IProtocolInf::unpack_recurcive() : " << e.what() << std::endl;
            res = false;
        }

        return res;
    }

    SegmentsType& get_segments(void) { return segments; }

    // fragment message. & make some segments.
    virtual bool pack(const void* msg_raw, size_t msg_size, enum_c::ServerType server_type) {
        std::cout << "[Error] IProtocolInf::pack() : undefined function." << std::endl;
        exit(-1);
    }

    // classify segment. & extract payloads. & combine payloads. => make One-payload.
    virtual bool unpack(const void* msg_raw, size_t msg_size) {
        std::cout << "[Error] IProtocolInf::unpack() : undefined function." << std::endl;
        exit(-1);
    }

protected:
    SegmentsType segments;   // packed messages.

};


class CNoneProtocol : public IProtocolInf {
public:
    CNoneProtocol(void);

    ~CNoneProtocol(void);

    bool pack(const void* msg_raw, size_t msg_size, enum_c::ServerType server_type) override;

    bool unpack(const void* msg_raw, size_t msg_size) override;

};

#endif // INTERFACE_PROTOCOL_H_