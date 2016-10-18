#include <vector>
#include <string>
#include <dirent.h>

#include "FileUtil.hpp"

/*
 * Implementação da classe que faz a leitura do diretório e do arquivo para envio
 */

std::vector<std::string> FileUtil::listDir(std::string dirName) {
	DIR *directory = opendir(dirName.c_str());
	if(!directory) throw DirectoryNotFoundException();

	struct dirent *entry;
	std::vector<std::string> result;

	while(entry = readdir(directory)) {
		result.push_back(entry->d_name);
	}

	return result;
};

/*
 * Implementação da exceção ocorrida quando o diretório não existe
 */
const char* DirectoryNotFoundException::what() const throw() {
	return "Diretório não encontrado";
}
