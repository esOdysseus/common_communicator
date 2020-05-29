/***
 * util.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <cassert>
#include <random>
#include <ctime>

#include <string.h>
#include <errno.h>
#include <stdexcept>
#ifdef _WIN32
#include <direct.h>		//mkdir
#else
#include <sys/stat.h>	// mkdir
#endif
#include <stdio.h>

#include <logger.h>
#include <util.h>


int makedirs(const char *_path_, __mode_t permission) {
	int ret = -1;
	char* path = NULL;
	size_t length=0;
	assert(_path_ != NULL);
	assert(*_path_ == '/');

	try {
		length = strlen(_path_);
		assert(length >= 3);

		path = new char[length];
		strncpy(path, _path_, length);

		for(size_t i=2; i < length; i++) {
			if( *(path+i) == (char)NULL ) {
				break;
			}

			if( *(path+i) == '/' ) {

				*(path+i) = (char)NULL;
				ret = mkdir(path, permission);
				*(path+i) = '/';
				if( ret != 0 && errno != EEXIST) {
					switch(errno) {
					case EAGAIN:
						*(path+i) = (char)NULL;
						mkdir(path, permission);
						*(path+i) = '/';
						break;
					default:
						throw std::runtime_error("makedirs() is failed.");
						break;
					}
				}	// if

			}	// if
		}	// for

		ret = mkdir(_path_, permission);
	}
	catch( const std::exception &e ) {
		LOGERR("[%s] %d: %s\n", e.what(), errno, strerror(errno));
		ret = -1;
	}

	if( path != NULL) {
		delete [] path;
		path = NULL;
	}

	return ret;
}


uint32_t gen_random_num(uint32_t _min_, uint32_t _max_) {
	uint32_t value = -1;
	std::mt19937 gen( (uint32_t)time(NULL) );
    std::uniform_int_distribution<uint32_t> dist(_min_, _max_);

    value = dist( gen );
    assert(_min_ <= value && value <= _max_);
    return value;
}



