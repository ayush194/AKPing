/*
	Author: Ayush Kumar
	Date: April 20, 2020
*/

// A simple C++ program to send pings to a given hostname
// To build:
//      make all
// To run:
//      ./AKPing <hostname>
// To clean:
//		make clean
  
#include <stdio.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h> 
#include <unistd.h> 
#include <string.h> 
#include <stdlib.h> 
#include <netinet/ip.h>
#include <netinet/ip_icmp.h> 
#include <time.h> 
#include <fcntl.h> 
#include <signal.h> 
#include <time.h>
#include <string>
#include <iostream>

class Ping {
	typedef struct sockaddr sockaddr;
	typedef struct sockaddr_in sockaddr_in;
	typedef struct in_addr in_addr;
	typedef struct hostent hostent;
	typedef struct timespec timespec;
	typedef struct timeval timeval;
	// icmp header structure 
	typedef struct icmphdr {
		// message type
	  	u_int8_t type;
	  	// message type sub-code
	  	u_int8_t code;
	  	u_int16_t checksum;
		union {
			// echo datagram
			struct {
				u_int16_t id;
				u_int16_t sequence;
			} echo;
			// gateway address
			u_int32_t gateway;
			struct {
				// u_int16_t __unused;
				// path mtu discovery
				u_int16_t mtu;
			} frag;
		} un;
	} icmphdr;

private:
	u_int _port;
	u_int _sleep_time;
	u_int _receive_timeout;
	u_int _sockfd;
	sockaddr_in _addr_con;
	std::string _ip_addr;
	std::string _hostname;
	std::string _message;
	static bool _ping_flag;
	bool _dnsLookup();
	void _ping();
	u_short _checksum(void*, int);
	static void _signalHandler(int);
	static void (*_userSignalHandler) (int);

public:
	Ping(const std::string& hostname, int p = 0, int st = 1000000, int rt = 1):  
		_port(p), _sleep_time(st), _receive_timeout(rt), _hostname(hostname) {
		Ping::_ping_flag = true;
	}
	void ping(const std::string& = std::string("Hello from the other side!"));
	void registerSignalHandler(void(*signalHandler)(int) = NULL);
	void* createNewPacket(u_int = 0);
	inline u_int getPacketSize() {return this->_message.size() + sizeof(icmphdr) + 1;}
};