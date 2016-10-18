#include <vector>
#include <string>
#include <iostream>
#include "FileUtil.hpp"

int main(int argc, char* argv[]) {
	FileUtil f;

	std::vector<std::string> folders;
	try {
		folders = f.listDir(argv[1]);
	} catch(DirectoryNotFoundException& e) {
		std::cout <<" asddasdas\n";
		return 0;
	}

	for(int i = 0; i < folders.size(); i++) {
		std::string val = folders[i];
		std::cout << val << "\n";
	}

	return 0;
}
