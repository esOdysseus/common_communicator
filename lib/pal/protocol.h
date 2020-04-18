/***
 * protocol.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdint.h>

__BEGIN_DECLS

/*
 * Value for the pt_module_t.tag field
 */
#define MAKE_TAG_CONSTANT(A,B,C,D) (((A) << 24) | ((B) << 16) | ((C) << 8) | (D))
#define PROTOCOL_MODULE_TAG MAKE_TAG_CONSTANT('P', 'T', 'M', 'T')
#define PROTOCOL_CONTEXT_TAG MAKE_TAG_CONSTANT('P', 'T', 'D', 'T')
struct pt_module_t;
struct pt_module_methods_t;
struct pt_standard_t;

typedef enum E_LANGUAGE {
    E_LANGUAGE_C = 1,
    E_LANGUAGE_CPP = 2
} E_LANGUAGE;

/**
 * Every protocol module must have a data structure named PAL_MODULE_INFO_SYM
 * and the fields of this data structure must begin with pt_module_t
 * followed by module specific information.
 */
typedef struct pt_module_t {
    /** tag must be initialized to PROTOCOL_MODULE_TAG */
    uint32_t tag;
    /** standard-pal API version number for the module */
    uint16_t pal_api_version;
    /** major version number for the module */
    uint16_t version_major;
    /** minor version number of the module */
    uint16_t version_minor;
    /** Identifier of module */
    const char *id;
    /** Name of this module */
    const char *name;
    /** Author/owner/implementor of the module */
    const char *author;
    /** Modules methods */
    struct pt_module_methods_t* methods;
    /** module's dso */
    void* dso;
    /** padding to 128 bytes, reserved for future use */
    uint32_t reserved[32-7];
} pt_module_t;

typedef struct pt_module_methods_t {
    /** Open a specific protocol_contexts */
    int (*open)(const struct pt_module_t* module, const char* id,
            struct pt_standard_t** handler);
} pt_module_methods_t;

/**
 * Every protocol_contexts data structure must begin with pt_standard_t
 * followed by module specific public methods and attributes.
 */
typedef struct pt_standard_t {
    /** tag must be initialized to PROTOCOL_CONTEXT_TAG */
    uint32_t tag;
    /** version number for pt_standard_t */
    uint32_t version;
    /** supported language type */
    E_LANGUAGE language;
    /** reference to the module this protocol_contexts belongs to */
    struct pt_module_t* module;
    /** padding reserved for future use */
    uint32_t reserved[12];
    /** Close this protocol_contexts */
    int (*close)(struct pt_standard_t* handler);
} pt_standard_t;

typedef struct pt_multi_module_t {
    /**
     * Common methods of the multiple module. This *must* be the first member
     * of multi_module as users of this structure will cast a pt_module_t
     * to multi_module pointer in contexts where it's known
     * the pt_module_t references a multi_module.
     */
	unsigned int mod_cnt;
    struct pt_module_t* module;
} pt_multi_module_t;

/**
 * Name of the pal_module_info
 */
#define PAL_MODULE_INFO_SYM         PMI

/**
 * Name of the pal_module_info as a string
 */
#define PAL_MODULE_INFO_SYM_AS_STR  "PMI"

/**
 * Get the module info associated with a module by id.
 * @return: 0 == success, <0 == error and *pHmi == NULL
 */
int pt_get_module(const char *id, const char *path, const struct pt_module_t **module);


__END_DECLS

#endif /* PROTOCOL_H_ */
