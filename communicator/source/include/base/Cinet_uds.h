/***
 * Cinet_uds.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef _UDS_INET_MANAGER_H_
#define _UDS_INET_MANAGER_H_

#include <map>
#include <string>
#include <memory>
#include <mutex>

#include <netinet/in.h>     /* for sockaddr_in */
#include <sys/un.h>         /* for sockaddr_un */

#include <Enum_common.h>


#define UDS_DIR			"/tmp/cmcomm/uds/"	// Max=42 byte
#define UDS_PATH_MAXSIZE	108U
#define UDS_SERVER      "server"
#define UDS_CLIENT      "client%u"
#define UDS_SERVER_FORM		UDS_DIR "%s:%u_" UDS_SERVER	// {UDS_DIR}{alias_id}:{instance_num}_server // size 107 byte
                                        		        // alias_id(Max=40byte) + instance_num(Max=5byte) + random_num(Max=12byte) + dumy(Min=26byte)
#define UDS_CLIENT_FORM		UDS_DIR "%s:%u_" UDS_CLIENT // {UDS_DIR}{alias_id}:{instance_num}_client{dumy_number} // size 107 byte
                                        	            // alias_id(Max=40byte) + instance_num(Max=5byte) + random_num(Max=12byte) + dumy(Min=26byte)


/**************
 * IP/Port info
 */
class Cipport {
public:
	static constexpr uint32_t IP_SIZE = UDS_PATH_MAXSIZE;
	char* 		ip;
	uint16_t	port;

	Cipport(void) {
		clear();
		ip = new char[IP_SIZE];
	}

	~Cipport(void) {
		if( ip != NULL ) {
			delete [] ip;
		}
		clear();
	}

private:
	void clear(void) {
		ip = NULL;
		port = 0;
	}

};

/**************
 * INET/UDS manager
 */
class Cinet_uds {
public:
    using PVDM = enum_c::ProviderMode;

public:
    Cinet_uds(int domain, int sock_type, sa_family_t addr_type);

    ~Cinet_uds(void);

    std::shared_ptr<Cipport> get_ip_port(const struct sockaddr_in &addr);

    static std::shared_ptr<Cipport> get_ip_port(const struct sockaddr_un &addr);

    void set_ip_port(struct sockaddr_in &addr, 
                     std::string &ip, uint16_t &port, 
                     PVDM mode=PVDM::E_PVDM_CLIENT);

    void set_ip_port(struct sockaddr_un &addr, 
                     std::string &ip, uint16_t &port, 
                     PVDM mode=PVDM::E_PVDM_CLIENT);

    /** socket processing */
    int Socket(int opt_flag=1);

    void Close(int sock_fd);

    int Bind(int sock_fd, struct sockaddr_in &addr);

    int Bind(int sock_fd, struct sockaddr_un &addr);

    int Bind_uds_client(int sock_fd, std::string ip=std::string(), uint16_t port=0);

    int Listen(int sock_fd, int concurrent_peer);

    int Accept (int sock_fd, struct sockaddr_in &addr);

    int Accept (int sock_fd, struct sockaddr_un &addr);

    int Connect (int sock_fd, struct sockaddr_in &addr);

    int Connect (int sock_fd, struct sockaddr_un &addr);

private:
    Cinet_uds(void) = delete;

    void clear(void);

    bool check_validation(void);

    bool check_port_available(struct sockaddr *addr, socklen_t length, const char *uds_file_path);

    void unlink_uds_file(std::string file_path);

    std::string get_ipv4addr( void );

private:
    int _sock_domain_;

    int _sock_type_;

    sa_family_t _addr_type_;

    std::mutex _mtx_uds_map_;   // for _uds_map_

    std::map<int /*socket-fd*/, std::string /*uds-path*/> _uds_map_;

    static constexpr const char* DEF_UDS_ID =	"localhost";

};


#endif // _UDS_INET_MANAGER_H_