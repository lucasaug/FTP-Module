#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <dirent.h>
#include <fstream>

#include "FileUtil.hpp"

/*
 * Implementação da classe que faz a leitura do diretório e do arquivo para envio
 */

/*
 * Inicializa o arquivo para leitura
 */
void FileUtil::initialize(std::string fileName, std::string directory) {
    std::string loc = directory;
    if (loc.back() != '/') loc.append("/");
    loc.append(fileName);

    this->file.open(loc, std::ios_base::binary);
    if (!file) throw FileNotFoundException();
};

/*
 * Listagem de diretórios
 */
std::vector<std::string> FileUtil::listDir(std::string dirName) {
    DIR *directory = opendir(dirName.c_str());
    if (!directory) throw DirectoryNotFoundException();

    struct dirent *entry;
    std::vector<std::string> result;

    while (entry = readdir(directory)) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            result.push_back(entry->d_name);
    }

    std::sort(result.begin(), result.end());

    closedir(directory);

    return result;
};

/*
 * Leitura de arquivo
 * Requer primeiro que a função initialize seja chamada
 */
bool FileUtil::readFile(char* buffer, int maxBufferSize) {
    memset(buffer, 0, maxBufferSize);
    return this->file.read(buffer, maxBufferSize);
};

/*
 * Determina se o arquivo chegou ao fim
 */
bool FileUtil::eof() {
    return this->file.eof();
}

/*
 * Fecha o arquivo
 */
void FileUtil::close() {
    this->file.close();
}

/*
 * Implementação da exceção ocorrida quando o diretório não existe
 */
const char* DirectoryNotFoundException::what() const throw() {
    return "Diretório não encontrado";
};

/*
 * Implementação da exceção ocorrida quando o arquivo não existe
 */
const char* FileNotFoundException::what() const throw() {
    return "Arquivo solicitado não encontrado";
};
