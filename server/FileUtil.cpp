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

void FileUtil::initialize(std::string fileName) {
    this->file.open(fileName, std::ios::binary);
    if (!file) throw FileNotFoundException();
};


std::vector<std::string> FileUtil::listDir(std::string dirName) {
    DIR *directory = opendir(dirName.c_str());
    if(!directory) throw DirectoryNotFoundException();

    struct dirent *entry;
    std::vector<std::string> result;

    while(entry = readdir(directory)) {
        result.push_back(entry->d_name);
    }

    std::sort(result.begin(), result.end());

    closedir(directory);

    return result;
};


bool FileUtil::readFile(char* buffer, int maxBufferSize) {
    memset(buffer, 0, maxBufferSize);
    this->file.read(buffer, maxBufferSize);
    return true;
};

bool FileUtil::eof() {
    return this->file.eof();
}

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
