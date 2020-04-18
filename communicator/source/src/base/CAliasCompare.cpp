/***
 * CAliasCompare.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <cassert>
#include <netinet/in.h>

#include <logger.h>
#include <CAliasCompare.h>


bool CALIAS_CMPFUNC_for_sockaddr_in::operator()(const struct sockaddr_in &left, const struct sockaddr_in &right) const {
    assert( left.sin_family == right.sin_family );
    double left_standard = (double)(left.sin_addr.s_addr) + ((double)(left.sin_port) / 100000.0);
    double right_standard = (double)(right.sin_addr.s_addr) + ((double)(right.sin_port) / 100000.0);
    return left_standard < right_standard;
}
