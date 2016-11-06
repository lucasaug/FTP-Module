#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "FileUtil.hpp"

#define MAXPENDING 10

/*
 * Essa função envia dados relativos à conexão entre os processos
 * byte a byte
 */
int safeSend(int sock, const char* data, int length) {
    int sendSuccess = 1;

    while (length-- && sendSuccess) {
        sendSuccess = send(sock, data, 1, 0);
    }

    if (sendSuccess) sendSuccess = send(sock, "\0", 1, 0);

    return sendSuccess;
}

/*
 * Essa função envia dados relativos à conexão entre os processos
 * byte a byte
 */
const char* safeRecv(int sock) {
    char buffer[2];
    std::string result;

    recv(sock, buffer, 1, 0);
    while (buffer[0] != '\0') {
        buffer[1] = '\0';
        result.append(buffer);
        recv(sock, buffer, 1, 0);
    }

    return result.c_str();
}

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
        close(serverSocket);
        return -1;
    }

    if (listen(serverSocket, MAXPENDING) < 0) {
        reportError(-4, "Erro de listen");
        close(serverSocket);
        return -1;
    }

    return serverSocket;
}

void processConnection(int socket, int bufferSize, std::string directory) {
    char* buffer = (char*) malloc(sizeof(char) * bufferSize);
    std::string command = "";
    int numCharacters;

    const char* nChars = safeRecv(socket);
    numCharacters = atoi(nChars);

    while (command.size() < numCharacters) {
        recv(socket, buffer, bufferSize, 0);
        command.append(buffer);
    }

    int sendSuccess = 0;
    std::string option = command.substr(0, 1);

    FileUtil f;
    if (option == "G") {
        std::string fileName = command.substr(2);

        try {
            f.initialize(fileName);
            f.readFile(buffer, bufferSize);

            sendSuccess = send(socket, "1", 1, 0);

            while (!f.eof() && sendSuccess >= 0) {
                sendSuccess = send(socket, buffer, bufferSize, 0);
                f.readFile(buffer, bufferSize);
            }

            sendSuccess = send(socket, buffer, bufferSize, 0);

            f.close();
        } catch (FileNotFoundException e) {
            // Envia um valor inicial indicando a existência do arquivo
            sendSuccess = send(socket, "0", 1, 0);
        }

    } else if (option == "L") {
        std::vector<std::string> files = f.listDir(directory);

        const char* numFiles = std::to_string(files.size()).c_str();
        sendSuccess = safeSend(socket, numFiles, strlen(numFiles));

        for (int i = 0; i < files.size() && sendSuccess >= 0; i++) {
            const char* fileSize = std::to_string(files[i].size()).c_str();
            sendSuccess = safeSend(socket, fileSize, strlen(fileSize));
            int sent = 0;

            while (sent < files[i].size() && sendSuccess >= 0) {
                int sendSize = bufferSize;
                if (files[i].size() - sent < bufferSize) sendSize = files[i].size() - sent;
                sendSuccess = send(socket, files[i].substr(sent, sendSize).c_str(), bufferSize, 0);

                sent += bufferSize;
            }
        }

    }

    if (sendSuccess < 0) {
        reportError(-7, "Erro de comunicação com servidor/cliente");
        free(buffer);
        return;
    }

    free(buffer);
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
        } else {
            processConnection(clientSocket, bufferSize, directory);
            shutdown(clientSocket, SHUT_WR);
            close(clientSocket);
        }
    }

    close(serverSocket);

    return 0;
}
