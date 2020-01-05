#ifndef _PROTOCOL_ABSTRACTION_LAYER_H_
#define _PROTOCOL_ABSTRACTION_LAYER_H_

#include <list>
#include <string>
#include <memory>

#include <pal/protocol.h>
#include <IProtocolInf.h>

#define PAL_API_VERSION	100

#define SAMPLE_PROTOCOLS_MODULE_PAL_ID 		"SAMPLE_PROTOCOLS"
#define SAMPLE_PROTOCOLS_MODULE_API_VERSION_MAJOR		(1)
#define SAMPLE_PROTOCOLS_MODULE_API_VERSION_MINOR		(1)
#define SAMPLE_PROTOCOLS_MODULE_API_VERSION		(SAMPLE_PROTOCOLS_MODULE_API_VERSION_MAJOR*10 + SAMPLE_PROTOCOLS_MODULE_API_VERSION_MINOR)

    
typedef struct cpp_protocol_lib_t {
   // protocol-context API.
   struct pt_standard_t common;

    /** Get available protocol_contexts */
    std::shared_ptr<std::list<std::string>> (*get_available_protocols)(void);
    /** Create instance of protocol correspond with protocol-name. */
    std::shared_ptr<IProtocolInf> (*create_instance)(std::string protocol_name);
} cpp_protocol_lib_t;


#endif // _PROTOCOL_ABSTRACTION_LAYER_H_