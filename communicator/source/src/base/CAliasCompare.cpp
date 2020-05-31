/***
 * CAliasCompare.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <cassert>
#include <memory>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>

#include <logger.h>
#include <CAliasCompare.h>
#include <Cinet_uds.h>

template struct CALIAS_CMPFUNC<struct sockaddr_in>;
template struct CALIAS_CMPFUNC<struct sockaddr_un>;

template <>
bool CALIAS_CMPFUNC<struct sockaddr_in>::operator()(const struct sockaddr_in &left, const struct sockaddr_in &right) const {
    assert( left.sin_family == right.sin_family );
    double left_standard = (double)(left.sin_addr.s_addr) + ((double)(left.sin_port) / 100000.0);
    double right_standard = (double)(right.sin_addr.s_addr) + ((double)(right.sin_port) / 100000.0);
    return left_standard < right_standard;
}

template <>
bool CALIAS_CMPFUNC<struct sockaddr_un>::operator()(const struct sockaddr_un &left, const struct sockaddr_un &right) const {
    double left_standard = 0.0;
    double right_standard = 0.0;
    std::shared_ptr<Cipport> left_ipport;
    std::shared_ptr<Cipport> right_ipport;
    assert( left.sun_family == right.sun_family );

    left_ipport = Cinet_uds::get_ip_port(left);
    right_ipport = Cinet_uds::get_ip_port(right);
    assert( left_ipport.get() != NULL );
    assert( right_ipport.get() != NULL );

    left_standard = (double)(inet_addr(left_ipport->ip)) + ((double)(left_ipport->port) / 100000.0);
    right_standard = (double)(inet_addr(right_ipport->ip)) + ((double)(right_ipport->port) / 100000.0);
    return left_standard < right_standard;
}
