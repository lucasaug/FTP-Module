#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "FileUtil.hpp"

#define MAXPENDING 10

void reportError(int code, std::string msg) { 
	std::cout << "Erro: " << code << " - Descrição: " << msg << "\n";
}

int configureSocket(in_port_t port) {
	int serverSocket;
	if ((serverSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		reportError(-2, "Erro de criação de socket");
		return -1;
	}

	struct sockaddr_in6 addr;
	addr.sin6_family = AF_INET6;
	addr.sin6_addr = in6addr_any;
	addr.sin6_port = htons(port);

	if (bind(serverSocket, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
		reportError(-3, "Erro de bind");
		return -1;
	}

	if (listen(serverSocket, MAXPENDING) < 0) {
		reportError(-4, "Erro de listen");
		return -1;
	}

	return serverSocket;
}

void processConnection(int socket, int bufferSize, std::string directory) {
	char* buffer = (char*) malloc(sizeof(char) * bufferSize);
	std::string command = "";

	while (recv(socket, buffer, bufferSize, 0) > 0) {
		command.append(buffer);
	}

	std::cout << command << "\n";
/*
	FileUtil f;
	f.initialize();

	int size = 5000;
	int resultSize;

    std::ofstream file;
    file.open("../hello");

	while (f.readFile(argv[1], buffer, size, &resultSize)) {
    	file.write(buffer, resultSize);
	}

    file.close();
*/
	free(buffer);
	close(socket);
}

int main(int argc, char* argv[]) {
	in_port_t port;
	int bufferSize;
	std::string directory;

	if (argc < 4) {
		reportError(-1, "Erros nos argumentos de entrada");
		return -1;
	}

	port = atoi(argv[1]);
	bufferSize = atoi(argv[2]);
	directory = std::string(argv[3]);

	int serverSocket = configureSocket(port);
	if (serverSocket == -1) return -1;

	while (1) {
		struct sockaddr_in6 clientAddr;
		socklen_t cAddrLen = sizeof(clientAddr);

		int clientSocket = accept(serverSocket, (struct sockaddr*) &clientAddr, &cAddrLen);
		if (clientSocket < 0) {
			reportError(-5, "Erro de accept");
		}

		processConnection(clientSocket, bufferSize, directory);
	}

	return 0;
}
