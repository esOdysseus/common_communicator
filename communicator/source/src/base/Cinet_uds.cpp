/***
 * Cinet_uds.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <cassert>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/socket.h>

#include <logger.h>
#include <util.h>
#include <Cinet_uds.h>

constexpr const char* Cinet_uds::DEF_UDS_ID;

/**************************
 * Public Member-Function.
 */
Cinet_uds::Cinet_uds(int domain, int sock_type, sa_family_t addr_type) {
    clear();

	_sock_domain_ = domain;
	_sock_type_ = sock_type;
    _addr_type_ = addr_type;
    if( check_validation() == false ) {
        throw std::invalid_argument("validation checking is failed.");
    }
}

Cinet_uds::~Cinet_uds(void) {
	std::map<int /*socket-fd*/, std::string /*uds-path*/>::iterator target;

    for(auto itor=_uds_map_.begin(); itor != _uds_map_.end();){
		target = itor;
		itor++;
		Close(target->first);
    }
    clear();
}

std::shared_ptr<Cipport> Cinet_uds::get_ip_port(const struct sockaddr_in &addr) {
	std::shared_ptr<Cipport> res;

	try {
		res = std::make_shared<Cipport>();
		res->port = ntohs(addr.sin_port);
		inet_ntop(_addr_type_, &addr.sin_addr.s_addr, res->ip, res->IP_SIZE);
		LOGD("IP/PORT=%s/%d\n", res->ip, res->port);
	}
	catch( const std::exception &e ) {
		LOGERR("%s", e.what());
		res.reset();
	}

	return res;
}

std::shared_ptr<Cipport> Cinet_uds::get_ip_port(const struct sockaddr_un &addr) {
	const char* start_pos = NULL;
	char* substr = NULL;
	std::shared_ptr<Cipport> res;

	if( strlen(addr.sun_path) <= 0 ) {
		return res;
	}

	/******
	 * Assumption
	 * addr.sun_path ==> /xxx/xxx/xxx/${IP}:${Port}_xxxx
	 */
	try {
		LOGD("Client socket file-path: len=%lu, %s\n", strlen(addr.sun_path), addr.sun_path);
		start_pos = strrchr(addr.sun_path, '/') + 1;
		assert( (uint8_t)(*start_pos) != (uint8_t)NULL );

		// make Cipport
		res = std::make_shared<Cipport>();
		strcpy(res->ip, start_pos);
		strtok_r(res->ip, "_", &substr);
		strtok_r(res->ip, ":", &substr);	// substr -> Port
		res->port = atoi(substr);
		LOGD("IP/PORT=%s/%d\n", res->ip, res->port);
	}
	catch( const std::exception &e ) {
		LOGERR("%s", e.what());
		res.reset();
	}

	return res;
}

void Cinet_uds::set_ip_port(struct sockaddr_in &addr, const char* ip, uint16_t &port, PVDM mode) {
	try {
		if( port == 0 ) {
			port = gen_random_num(1025, 65534);
		}

		addr.sin_family = _addr_type_;
		addr.sin_port = htons(port);
		if( ip == NULL ) {
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
		} else {
			addr.sin_addr.s_addr = inet_addr(ip);
		}
		// set all bits of the padding field to 0
		memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));
	}
	catch( const std::exception &e ) {
		LOGERR("%s", e.what());
		throw ;
	}
}

void Cinet_uds::set_ip_port(struct sockaddr_un &addr, const char* ip, uint16_t &port, PVDM mode) {	// for UDS
    if( ip == NULL ) {
        ip = DEF_UDS_ID;
    }

	try {
		if( port == 0 ) {
			port = gen_random_num(1, 65534);
		}
	
		// make address
		memset( &addr, '\0', sizeof( addr));
		addr.sun_family  = _addr_type_;
		switch( mode ) {
		case PVDM::E_PVDM_CLIENT:
			snprintf( addr.sun_path, UDS_PATH_MAXSIZE, UDS_CLIENT_FORM, ip, port, gen_random_num());
			break;
		case PVDM::E_PVDM_BOTH:
		case PVDM::E_PVDM_SERVER:
			snprintf( addr.sun_path, UDS_PATH_MAXSIZE, UDS_SERVER_FORM, ip, port);
			break;
		default:
			{
				std::string err_str = "Not supported mode.(" + std::to_string((uint32_t)mode) + ")";
				throw std::invalid_argument(err_str);
			}
		}

		// make directory & check port availability
		makedirs(UDS_DIR, 0755);
		LOGD("File-Path=%s\n", addr.sun_path);
	}
	catch( const std::exception &e ) {
		LOGERR("%s", e.what());
		throw ;
	}
}

int Cinet_uds::Socket(int opt_flag) {
	int sock_fd = 0;

	sock_fd = socket( _sock_domain_, _sock_type_, 0);
	if( -1 == sock_fd)
	{
		LOGERR("%d: %s\n", errno, strerror(errno));
		return -1;
	}

	if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt_flag, sizeof(opt_flag)) == -1) {
		LOGERR("%d: %s\n", errno, strerror(errno));
		return -1;
	}

	return sock_fd;
}

void Cinet_uds::Close(int sock_fd) {
	std::unique_lock<std::mutex> guard(_mtx_uds_map_);
    if( _uds_map_.find(sock_fd) != _uds_map_.end() ) {
        unlink_uds_file(_uds_map_[sock_fd]);
        _uds_map_.erase(sock_fd);
    }

	close(sock_fd);
}

int Cinet_uds::Bind(int sock_fd, struct sockaddr_in &addr) {
	return bind( sock_fd, (struct sockaddr*)&addr, sizeof(addr) );
}

int Cinet_uds::Bind(int sock_fd, struct sockaddr_un &addr) {
	char* type = strrchr(addr.sun_path, '_') + 1;
	assert(type != NULL);

	if( std::string(type) == UDS_SERVER ) {
		if( check_port_available((struct sockaddr*)&addr, sizeof(addr), addr.sun_path) == false) {
			return -1;
		}
	}

	std::unique_lock<std::mutex> guard(_mtx_uds_map_);
    _uds_map_.insert({sock_fd, std::string(addr.sun_path)});
	return bind( sock_fd, (struct sockaddr*)&addr, sizeof(addr) );
}

int Cinet_uds::Bind_uds_client(int sock_fd, const char* ip, uint16_t port) {
    struct sockaddr_un cliaddr;

    set_ip_port(cliaddr, ip, port);
    return Bind(sock_fd, cliaddr);
}

int Cinet_uds::Listen(int sock_fd, int concurrent_peer) {
	return listen(sock_fd, concurrent_peer);
}

int Cinet_uds::Accept (int sock_fd, struct sockaddr_in &addr) {
	socklen_t addr_size = sizeof(addr);

	memset((void*)&addr, '\0', addr_size);
	return accept( sock_fd, (struct sockaddr*)&addr, &addr_size);
}

int Cinet_uds::Accept (int sock_fd, struct sockaddr_un &addr) {
	socklen_t addr_size = sizeof(addr);

	memset((void*)&addr, '\0', addr_size);
	return accept( sock_fd, (struct sockaddr*)&addr, &addr_size);
}

int Cinet_uds::Connect (int sock_fd, struct sockaddr_in &addr) {
	return connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr));
}

int Cinet_uds::Connect (int sock_fd, struct sockaddr_un &addr) {
	return connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr));
}


/**************************
 * Private Member-Function.
 */
void Cinet_uds::clear(void) {
	_sock_domain_ = PF_UNSPEC;
	_sock_type_ = 0;
    _addr_type_ = AF_UNSPEC;
	std::unique_lock<std::mutex> guard(_mtx_uds_map_);
    _uds_map_.clear();
}

bool Cinet_uds::check_validation(void) {
    if( _sock_domain_ <= PF_UNSPEC) {
        return false;
    }

    if( _sock_type_ <= 0 ) {
        return false;
    }

    if( _addr_type_ <= AF_UNSPEC ) {
        return false;
    }

    return true;
}

bool Cinet_uds::check_port_available(struct sockaddr *addr, socklen_t length, const char *uds_file_path) {	// for UDS
	/** It's only for server-socket test. */
	int clisock = -1;
	assert(uds_file_path != NULL);

	if ( _sock_type_ == SOCK_STREAM ) {
		clisock  = socket( _sock_domain_, _sock_type_, 0);
		if( -1 == clisock)
		{
			LOGERR("%d: %s\n", errno, strerror(errno));
			return false;
		}

		if( -1 != connect( clisock, addr, length ) )
		{
			close( clisock);
			return false;
		}

		shutdown(clisock, SHUT_RDWR);	// Rx,Tx pending.
		close( clisock);	// socket destroy.
	}

    unlink_uds_file( uds_file_path );
	return true;
}

void Cinet_uds::unlink_uds_file(std::string file_path) {
    if( file_path.empty() ) {
        return ;
    }

    if ( 0 == access( file_path.data(), F_OK)) {
		LOGD("Remove UDS-file(%s)\n", file_path.data());
		unlink( file_path.data());
	}
}