#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include "FileUtil.hpp"


#include <cstdlib>

int main(int argc, char* argv[]) {
	FileUtil f;
	f.initialize();

	int size = 5000;
	char* buffer = (char*) malloc(sizeof(char) * size);

    std::ofstream file;
    file.open("../hello");

	while (f.readFile(argv[1], buffer, size)) {
    	file.write(buffer, size);
	}

    file.close();

	free(buffer);

	return 0;
}
