#include "Ping.h"

bool Ping::_ping_flag = true;
void (*Ping::_userSignalHandler) (int) = NULL;

void Ping::registerSignalHandler(void(*signalHandler)(int)) {
	Ping::_userSignalHandler = signalHandler;
	signal(SIGINT, &(Ping::_signalHandler));
}

void Ping::_signalHandler(int a) {
	if (Ping::_userSignalHandler) (*(Ping::_userSignalHandler))(a);
	Ping::_ping_flag = false;
}
  
u_short Ping::_checksum(void *bytes, int len) {
	u_short *buf = (u_short*)bytes; 
	u_int sum = 0; 
	u_short result; 
	for (sum = 0; len > 1; len -= 2) sum += *(buf++);
	if (len == 1) sum += *(u_char*)buf;
	sum = (sum >> 16) + (sum & 0xFFFF); 
	sum += (sum >> 16);
	result = ~sum;
	return result;
} 
  
bool Ping::_dnsLookup() {
	std::cout << "Resolving DNS..." << std::endl; 
	hostent* host_entity;
  
	if (!(host_entity = gethostbyname(this->_hostname.c_str()))) { 
		// No ip was found for the given hostname 
		return false; 
	}

	//filling up address structure
	this->_ip_addr = std::string(inet_ntoa(*(in_addr*)host_entity->h_addr));
	this->_addr_con.sin_family = host_entity->h_addrtype; 
	this->_addr_con.sin_port = htons(this->_port); 
	this->_addr_con.sin_addr.s_addr = *(long*)host_entity->h_addr; 
  
	return true;	  
}

void* Ping::createNewPacket(u_int msg_cnt) {
	u_int packet_size = this->getPacketSize();
	void* mem_location = malloc(packet_size);
	icmphdr* header = new(mem_location) icmphdr;
	bzero(header, sizeof(icmphdr));
	header->type = ICMP_ECHO;
	header->un.echo.id = getpid();
	strcpy((char*)mem_location + sizeof(icmphdr), this->_message.c_str());
	// null terminate the msg string
	*((char*)mem_location + packet_size - 1) = 0;
	header->un.echo.sequence = msg_cnt;
	header->checksum = this->_checksum(mem_location, packet_size);
	return mem_location;
}
  
void Ping::_ping() {
	int ttl = 64, addr_len = sizeof(sockaddr_in), msg_count = 0, packets_sent_cnt = 0, packets_received_cnt = 0;
	sockaddr_in src_addr;
	timespec start_time, end_time, tfs, tfe;
	timeval tv_out;
	tv_out.tv_sec = this->_receive_timeout;
	tv_out.tv_usec = 0;
	clock_gettime(CLOCK_MONOTONIC, &tfs);
	
	// set socket options at ip to TTL and value to 64
	#ifdef __APPLE__
	if (setsockopt(this->_sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl))) {
	#elif __linux__
	if (setsockopt(this->_sockfd, SOL_IP, IP_TTL, &ttl, sizeof(ttl))) {
	#endif
		std::cerr << "Setting socket options to TTL failed!" << std::endl;
		return;
	} else {
		std::cout << "Socket set to TTL.." << std::endl;
	}  
	// setting timeout of recv setting 
	setsockopt(this->_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof tv_out); 
	while(this->_ping_flag) {
		bool packet_sent = true, packet_received = true;
		void* packet = this->createNewPacket(msg_count);
		usleep(this->_sleep_time); 
  
		//send packet 
		clock_gettime(CLOCK_MONOTONIC, &start_time);
		if (sendto(this->_sockfd, packet, this->getPacketSize(), 0, (sockaddr*)&this->_addr_con, sizeof(this->_addr_con)) <= 0) {
			std::cerr << "Failed to deliver packet!" << std::endl;
			packet_sent = false; 
		} else {
			packets_sent_cnt++;
		}

		if (recvfrom(this->_sockfd, packet, this->getPacketSize(), 0, (sockaddr*)&src_addr, (socklen_t*)&addr_len) <= 0) {
			std::cerr << "Failed to receive packet!" << std::endl;
			packet_received = false;
		} else {
			clock_gettime(CLOCK_MONOTONIC, &end_time);   
			long double elapsed_time = ((long double)(end_time.tv_nsec - start_time.tv_nsec)) / 1000000.0;
			long double rtt_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0 + elapsed_time;

			if (packet_sent) {
				icmphdr* header = (icmphdr*)packet;
				if(!(header->type == 69 && header->code == 0)) { 
					std::cerr << "Error! Packet received with ICMP type " << header->type << " and code " << header->code << std::endl;
				} else {
					std::cout << this->getPacketSize() << " bytes received from " << this->_hostname.c_str() << "(IP = " << 
					this->_ip_addr.c_str() << "), ping_id = " <<  msg_count << ", TTL = " << ttl << ", RTT = " << rtt_time << " ms." << std::endl;
					packets_received_cnt++;
				}
			}
		}
		msg_count++;
	}
	clock_gettime(CLOCK_MONOTONIC, &tfe); 
	long double elapsed_time = ((long double)(tfe.tv_nsec - tfs.tv_nsec)) / 1000000.0;
	long double total_time = (tfe.tv_sec-tfs.tv_sec) * 1000.0 + elapsed_time;
	std::cout << "=============ping statistics=============" << std::endl;
	std::cout << packets_sent_cnt << " packets sent, " << packets_received_cnt << " packets received.\n";
	std::cout << ((packets_sent_cnt - packets_received_cnt)/packets_sent_cnt) * 100.0 << " \% packet loss.\n";
	std::cout << "Total time: " << total_time << " ms." << std::endl; 
}

void Ping::ping(const std::string& msg) {
	if (!(this->_dnsLookup())) {
		// DNS hostname resolution failed
		std::cerr << "DNS Error! Hostname could not be resolved!" << std::endl;
		return;
	}
	// DNS hostname resolved, ip address obtained
	std::cout << "Trying to connect to " << this->_hostname << " IP: " << this->_ip_addr << std::endl;
	// Create a new socket 
	if ((this->_sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
		std::cerr << "Socket could not be created!" << std::endl;
	} else {
		std::cout << "Socket " << this->_sockfd << " created successfully!" << std::endl;
	}
	this->_message = msg;
	this->_ping();
}