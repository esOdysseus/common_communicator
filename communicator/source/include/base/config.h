/***
 * config.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <string>

// for Common-API Version-Info.
#if !defined( VER_MAJ )
    #error "We need VER_MAJ for Common-API version."
#endif

#if !defined( VER_MIN )
    #error "We need VER_MIN for Common-API version."
#endif

#if !defined( VER_PAT )
    #error "We need VER_PAT for Common-API version."
#endif

#define STRING_OF_COMMON_API_VERSION      std::to_string(VER_MAJ) + '.' + std::to_string(VER_MIN) + '.' + std::to_string(VER_PAT)



#endif // _CONFIG_H_