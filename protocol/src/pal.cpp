/*
 * pal.cpp
 *
 *  Created on: 2020. 1. 1.
 *      Author: esOdysseus
 */

#include <cassert>

#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include <malloc.h>

#define LOGGER_TAG "PAL"
#include <pal.h>
#include <logger.h>
#include <protocol/CPBigEndian.h>
#include <protocol/CPLittleEndian.h>

namespace pal {

    typedef enum E_ERROR {
        E_NO_ERROR = 0,
        E_ITS_NOT_SUPPORTED_TYPE = 3,
        E_INVALID_VALUE = 4
    }E_ERROR;

    static const char* exception_switch(E_ERROR err_num) {
        switch(err_num) {
        case E_ERROR::E_NO_ERROR:
            return "E_NO_ERROR in PAL-pkg.";
        case E_ERROR::E_ITS_NOT_SUPPORTED_TYPE:
            return "E_ITS_NOT_SUPPORTED_TYPE in PAL-pkg.";
        case E_ERROR::E_INVALID_VALUE:
            return "E_INVALID_VALUE in PAL-pkg.";
        default:
            return "\'not support error_type\' in PAL-pkg.";
        }
    }
    #include <CException.h>

    static int sample_close(pt_standard_t *handler) {
        assert(handler != NULL);
        LOGD("Called");

        handler->module = NULL;
        handler->close = NULL;

        free(handler);
        return 1;
    }

    static std::shared_ptr<std::list<std::string>> sample_get_available_protocols(void) {
        using ProtocolList = std::list<std::string>;
        std::shared_ptr<ProtocolList> pt_list = std::make_shared<ProtocolList>();
        LOGD("Called");

        try{
            pt_list->push_back(CPBigEndian::Protocol_NAME);
            pt_list->push_back(CPLittleEndian::Protocol_NAME);
        }
        catch (const std::exception &e) {
            LOGERR("%s", e.what());
        }

        return pt_list;
    }

    static std::shared_ptr<IProtocolInf> sample_create_instance(std::string protocol_name) {
        assert(protocol_name.empty() == false);
        assert(protocol_name.length() > 0);
        LOGD("Called");

        try{
            if( protocol_name == CPBigEndian::Protocol_NAME) {
                std::shared_ptr<CPBigEndian> pt_inst = std::make_shared<CPBigEndian>();
                return pt_inst;
            }
            else if( protocol_name == CPLittleEndian::Protocol_NAME) {
                std::shared_ptr<CPLittleEndian> pt_inst = std::make_shared<CPLittleEndian>();
                return pt_inst;
            }
            else {
                throw CException(E_ERROR::E_ITS_NOT_SUPPORTED_TYPE);
            }
        }
        catch( const std::exception &e) {
            LOGERR("%s", e.what());
            throw e;
        }
    }

    static int sample_open(const pt_module_t* module, const char __attribute__((unused)) *id,
                           pt_standard_t** handler)
    {
        assert(module != NULL);
        assert(handler != NULL);
        LOGD("Called");

        cpp_protocol_lib_t *pcontext = (cpp_protocol_lib_t *)malloc(sizeof(cpp_protocol_lib_t));
        memset(pcontext, 0, sizeof(cpp_protocol_lib_t));

        pcontext->common.tag = PROTOCOL_CONTEXT_TAG;
        pcontext->common.version = SAMPLE_PROTOCOLS_MODULE_API_VERSION;
        pcontext->common.language = E_LANGUAGE::E_LANGUAGE_CPP;
        pcontext->common.module = (struct pt_module_t*) module;
        pcontext->common.close = sample_close;
        pcontext->get_available_protocols = sample_get_available_protocols;
        pcontext->create_instance = sample_create_instance;

        *handler = (pt_standard_t*)pcontext;
        return 1;
    }

    static struct pt_module_methods_t sample_module_methods = {
        .open = sample_open
    };


}   // pal


#define MODULE_CNT		1
struct pt_module_t module_list[MODULE_CNT] = {
    {
        .tag = PROTOCOL_MODULE_TAG,
        .pal_api_version    = PAL_API_VERSION,
        .version_major		= SAMPLE_PROTOCOLS_MODULE_API_VERSION_MAJOR,
        .version_minor		= SAMPLE_PROTOCOLS_MODULE_API_VERSION_MINOR,
        .id                 = SAMPLE_PROTOCOLS_MODULE_PAL_ID,
        .name               = "Demo Sample-Protocols-Module PAL",
        .author             = "eunseok.kim@mobis.co.kr",
        .methods            = &pal::sample_module_methods,
    }
};

pt_multi_module_t PAL_MODULE_INFO_SYM = {
    .mod_cnt = MODULE_CNT,
    .module  = module_list
};

