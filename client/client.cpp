#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
	char *ip_server = "localhost";
	char *message = "TESTANDO 123";
	in_port_t port = 1234;

	int sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in6 servAddr;

	memset(&servAddr, 0, sizeof(servAddr));

	servAddr.sin6_family = AF_INET6;

	int rtnVal = inet_pton(AF_INET6, ip_server, &servAddr.sin6_addr.s6_addr);

	servAddr.sin6_port = htons(port);

	connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr));

	size_t messageLen = strlen(message);

	ssize_t numBytes = send(sock, message, messageLen, 0);

	close(sock);
	return 0;
}
