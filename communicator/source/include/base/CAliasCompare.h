/***
 * CAliasCompare.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef _C_ALIAS_COMPARE_H_
#define _C_ALIAS_COMPARE_H_

template <typename ADDR_TYPE=struct sockaddr_in>
struct CALIAS_CMPFUNC {
    bool operator()(const ADDR_TYPE &left, const ADDR_TYPE &right) const;
};


#endif // _C_ALIAS_COMPARE_H_