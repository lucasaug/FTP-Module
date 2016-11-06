all:
	g++ -std=c++11 -c -o FileUtil.o -g FileUtil.cpp
	g++ -std=c++11 -o bin/servidorFTP  -g server.cpp FileUtil.o -lpthread 

clean:
	rm bin/servidorFTP
	rm *.o
