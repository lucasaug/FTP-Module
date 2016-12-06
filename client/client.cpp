#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>


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
std::string safeRecv(int sock) {
    char buffer[2];
    std::string result;

    recv(sock, buffer, 1, 0);
    // Lemos até encontrar um byte nulo
    while (buffer[0] != '\0') {
        result.append(buffer);
        recv(sock, buffer, 1, 0);
    }

    return result;
}

/*
 * Função que reporta um erro no formato especificado
 */
void reportError(int code, std::string msg) { 
    std::cout << "Erro: " << code << " - Descrição: " << msg << "\n";
}

/*
 * Cria um socket para o cliente
 */
int configureSocket(char* ip_server, in_port_t port) {
    int sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        reportError(-2, "Erro de criação de socket");
        return -1;
    }

    // Utilizamos as structs que suportam IPv6
    struct sockaddr_in6 servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin6_family = AF_INET6;
    int rtnVal = inet_pton(AF_INET6, ip_server, &servAddr.sin6_addr.s6_addr);
    servAddr.sin6_port = htons(port);

    if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
        reportError(-6, "Erro de connect");
        return -1;
    }

    return sock;
}

/*
 * Processa o comando list do lado do cliente, enviando a requisição
 * e recebendo os resultados
 */
void processList(int sock, int bufferSize) {
    // Envia o tamanho do comando e em seguida o comando list
    int sendSuccess = safeSend(sock, "1", 1);
    
    if (sendSuccess >= 0) sendSuccess = send(sock, "L", bufferSize, 0);

    if (sendSuccess < 0) {
        reportError(-7, "Erro de comunicação com servidor");
        return;
    }

    char* buffer = (char*) malloc(sizeof(char) * bufferSize);

    // Recebe a quantidade de arquivos retornados
    std::string entries = safeRecv(sock);

    int numEntries = atoi(entries.c_str());

    if (numEntries == 0) reportError(-999, "Diretório vazio");

    // Faz a leitura de cada nome de arquivo
    std::vector<std::string> result;
    while (numEntries--) {
        // Recebe o tamanho do nome desse arquivo
        std::string entryLen = safeRecv(sock);
        int size = atoi(entryLen.c_str());

        std::string entry = "";
        while (entry.size() < size) {
            recv(sock, buffer, bufferSize, 0);

            entry.append(buffer);
        }

        result.push_back(entry);
    }

    for (int i = 0; i < result.size(); i++) {
        std::cout << result[i] << "\n";
    }

    free(buffer);
}

/*
 * Processa o comando get do lado do cliente, enviando a requisição
 * e recebendo os resultados
 */
void processGet(int sock, char* fileName, int bufferSize) {
    // Inicia a medição do tempo gasto
    struct timeval begin, end;
    int err = gettimeofday(&begin, NULL);

    if (err) {
        reportError(-999, "Erro na função gettimeofday");
        return;
    }

    std::string message = "G ";
    message.append(fileName);

    // Envia o tamanho do comando
    const char* msgSize = std::to_string(message.size()).c_str();
    int sendSuccess = safeSend(sock, msgSize, strlen(msgSize));

    // Envia o comando
    unsigned int sent = 0;
    while (sent < message.size() && sendSuccess >= 0) {
        send(sock, message.substr(sent, bufferSize).c_str(), bufferSize, 0);
        sent += bufferSize;
    }

    if (sendSuccess < 0) {
        reportError(-7, "Erro de comunicação com servidor");
        return;
    }

    char* data   = (char*) malloc(sizeof(char) * bufferSize);
    char* buffer = (char*) malloc(sizeof(char) * bufferSize);

    // Checa se o arquivo existe
    recv(sock, buffer, 1, 0);

    if (buffer[0] == '0') {
        // Arquivo não existe
        reportError(-8, "Arquivo solicitado não encontrado");
        return;
    }

    // Mantemos dois buffers, um mais atualizado (buffer) que armazena
    // os dados à medida que são recebidos, e outro (data) que armazena
    // o bloco de dados recebido anteriormente
    // Isso é feito para que possamos remover os caracteres NUL do fim do
    // arquivo, já que esse tipo de evento pode corromper arquivos de texto
    unsigned int received = bufferSize;
    recv(sock, data, bufferSize, 0);

    std::ofstream file;
    file.open(fileName, std::ios_base::binary);

    while (recv(sock, buffer, bufferSize, 0) > 0) {
        file.write(data, bufferSize);
        memcpy(data, buffer, bufferSize);
        received += bufferSize;
    }

    file.write(data, strlen(data));

    file.close();
    free(buffer);
    free(data);

    err = gettimeofday(&end, NULL);

    if (err) {
        reportError(-999, "Erro na função gettimeofday");
        return;
    }

    // Tempo em milissegundos
    long double spent = (1000 * (end.tv_sec - begin.tv_sec) + (end.tv_usec - begin.tv_usec) / 1000);
    // E agora em segundos
    spent = spent / 1000;

    printf("Arquivo %s\tBuffer %5u byte, %10.2f kbps (%u bytes em %3u.%06u s)\n", fileName,
           (unsigned int) bufferSize, received / spent, received, (unsigned int) (end.tv_sec - begin.tv_sec),
           (unsigned int) (end.tv_usec - begin.tv_usec));
}

/*
 * Função principal - Lê os argumentos e chama a função adequada para
 * fazer a requisição e leitura dos resultados
 */
int main(int argc, char* argv[]) {
    if (argc < 5) {
        reportError(-1, "Erros nos argumentos de entrada");
        return -1;
    }

    if (strcmp(argv[1], "list") == 0) {
        if (argc > 5) {
            reportError(-1, "Erros nos argumentos de entrada");
            return -1;
        }
        int sock = configureSocket(argv[2], atoi(argv[3]));
        if (sock == -1) return -1;

        processList(sock, atoi(argv[4]));
        close(sock);
    } else if(strcmp(argv[1], "get") == 0) {
        if (argc < 6 || argc > 6) {
            reportError(-1, "Erros nos argumentos de entrada");
            return -1;
        }

        int sock = configureSocket(argv[3], atoi(argv[4]));
        if (sock == -1) return -1;

        processGet(sock, argv[2], atoi(argv[5]));
        close(sock);
    } else {
        reportError(-10, "Comando de clienteFTP não existente");
        return -1;
    }

    return 0;
}
