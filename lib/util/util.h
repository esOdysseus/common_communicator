/***
 * util.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef UTIL_H_
#define UTIL_H_

#include <bits/types.h>		/* for __mode_t and __dev_t.  */


int makedirs(const char *path, __mode_t permission);

uint32_t gen_random_num(uint32_t _min_=1, uint32_t _max_=4294967295);


#endif /* UTIL_H_ */