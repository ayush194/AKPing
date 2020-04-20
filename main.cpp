#include "Ping.h"

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cout << "Incorrect Input Format!\n";
		std::cout << "Format: " << argv[0] << " <hostname>" << std::endl;
		return 1;
	}

	// pass the hostname as an argument to the ping constructor
	Ping myping((std::string(argv[1])));
	myping.registerSignalHandler();
	// pass the message as an argument to the ping method
	myping.ping(std::string("Hello World!"));

	return 0; 
}