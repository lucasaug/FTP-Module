#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "FileUtil.hpp"

#define MAXPENDING 10

/*
 * Parâmetros para a thread
 */
struct parameters {
    int socket;
    int bufferSize;
    std::string directory;
};

/*
 * Essa função envia dados relativos à conexão entre os processos
 * byte a byte
 */
int safeSend(int sock, const char* data, int length) {
    int sendSuccess = 1;

    for (int i = 0; i < length && sendSuccess >= 0; i++) {
        sendSuccess = send(sock, data + i, 1, 0);
    }

    if (sendSuccess >= 0) sendSuccess = send(sock, "\0", 1, 0);

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
    // Lemos até encontrar um byte nulo
    while (buffer[0] != '\0') {
        buffer[1] = '\0';
        result.append(buffer);
        recv(sock, buffer, 1, 0);
    }

    return result.c_str();
}

/*
 * Função que reporta um erro no formato especificado
 */
void reportError(int code, std::string msg) { 
    std::cout << "Erro: " << code << " - Descrição: " << msg << "\n";
}

/*
 * Cria um socket para o servidor
 */
int configureSocket(in_port_t port) {
    int serverSocket;
    if ((serverSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        reportError(-2, "Erro de criação de socket");
        return -1;
    }

    // Utilizamos as structs que suportam IPv6
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

/*
 * Essa é a função chamada por cada nova thread criada
 * Recebe uma conexão e executa o comando recebido
 */
void *processConnection(void* params) {
    // Desempacota os parâmetros
    struct parameters *p = (struct parameters*) params;
    int socket = p->socket;
    int bufferSize = p->bufferSize;
    std::string directory = p->directory;

    char* buffer = (char*) malloc(sizeof(char) * bufferSize);
    std::string command = "";
    int numCharacters;

    // Primeiramente recebe a quantidade de caracteres no comando
    const char* nChars = safeRecv(socket);
    numCharacters = atoi(nChars);

    // Em seguida recebe o comando
    while (command.size() < numCharacters) {
        recv(socket, buffer, bufferSize, 0);
        command.append(buffer);
    }

    int sendSuccess = 1;
    std::string option = command.substr(0, 1);

    FileUtil f;
    if (option == "G") {
        // Comando GET
        std::string fileName = command.substr(2);

        try {
            // Inicializa e lê o arquivo
            f.initialize(fileName, directory);

            // Envia um valor inicial indicando a existência do arquvio
            sendSuccess = send(socket, "1", 1, 0);

            while (f.readFile(buffer, bufferSize) && sendSuccess >= 0) {
                sendSuccess = send(socket, buffer, bufferSize, 0);
            }

            f.close();
        } catch (FileNotFoundException e) {
            // Envia um valor inicial indicando a inexistência do arquivo
            sendSuccess = send(socket, "0", 1, 0);
        }

    } else if (option == "L") {
        // Comando LIST
        std::vector<std::string> files = f.listDir(directory);

        // Envia o número de arquivos obtidos
        const char* numFiles = std::to_string(files.size()).c_str();
        sendSuccess = safeSend(socket, numFiles, strlen(numFiles));

        for (int i = 0; i < files.size() && sendSuccess >= 0; i++) {
            const char* fileSize = std::to_string(files[i].size()).c_str();

            // Envia o tamanho da string com o nome do arquivo
            sendSuccess = safeSend(socket, fileSize, strlen(fileSize));
            int sent = 0;

            // Envia o nome do arquivo
            while (sent < files[i].size() && sendSuccess >= 0) {
                int sendSize = bufferSize;
                if (files[i].size() - sent < bufferSize) sendSize = files[i].size() - sent;
                sendSuccess = send(socket, files[i].substr(sent, sendSize).c_str(), bufferSize, 0);

                sent += bufferSize;
            }
        }

    }

    // Caso algum send tenha tido problemas, reporta-se o erro
    if (sendSuccess < 0) {
        reportError(-7, "Erro de comunicação com cliente");
    }


    shutdown(socket, SHUT_WR);
    close(socket);

    free(buffer);
}

/*
 * Função principal - Lê os argumentos e espera por conexões, criando novas
 * threads quando as recebe
 */
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
    // Retorna em caso de erro de criação do socket
    if (serverSocket == -1) return -1;

    while (1) {
        struct sockaddr_in6 clientAddr;
        socklen_t cAddrLen = sizeof(clientAddr);

        // Recebe conexão de cliente
        int clientSocket = accept(serverSocket, (struct sockaddr*) &clientAddr, &cAddrLen);
        if (clientSocket < 0) {
            reportError(-5, "Erro de accept");
        } else {
            // Cria nova thread passando os parâmetros necessários
            pthread_t thread;
            struct parameters p;
            p.socket = clientSocket;
            p.bufferSize = bufferSize;
            p.directory = directory;

            pthread_create(&thread, NULL, processConnection, (void*) &p);
        }
    }

    close(serverSocket);

    return 0;
}
