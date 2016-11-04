/*
 * Descreve a classe que faz a leitura do diretório e do arquivo para envio
 * Descreve também as exceções utilizadas
 */

#include <iostream>

#ifndef _FTP_FILE_UTIL
#define _FTP_FILE_UTIL

class FileUtil {
	private:
	std::streampos filePos;

	public:
	void initialize();
	std::vector<std::string> listDir(std::string);
	bool readFile(std::string, char*, int, int*);
};

class DirectoryNotFoundException: public std::exception {
	virtual const char* what() const throw();
};

class FileNotFoundException: public std::exception {
	virtual const char* what() const throw();
};

#endif
