/*
 * Descreve a classe que faz a leitura do diretório e do arquivo para envio
 * Descreve também as exceções utilizadas
 */

#ifndef _FTP_FILE_UTIL
#define _FTP_FILE_UTIL

class FileUtil {
	public:
	std::vector<std::string> listDir(std::string);
};


class DirectoryNotFoundException: public std::exception {
	virtual const char* what() const throw();
};

#endif
