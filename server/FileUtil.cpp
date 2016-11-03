#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <dirent.h>
#include <fstream>

#include "FileUtil.hpp"

/*
 * Implementação da classe que faz a leitura do diretório e do arquivo para envio
 */

void FileUtil::initialize() {
	this->filePos = 0;
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

	return result;
};


bool FileUtil::readFile(std::string fileName, char* buffer, int bufferSize) {
	if (this->filePos == -1) return false;

    std::ifstream file;
    file.open(fileName.c_str());

    file.seekg(this->filePos);

    file.read(buffer, bufferSize);

    this->filePos = file.tellg();

  	if (file.eof()) this->filePos = -1;

    file.close();

    return true;
};

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
