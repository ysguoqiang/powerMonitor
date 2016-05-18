powerMonitor: main.o threadPool.o
	g++ main.o threadPool.o -o powerMonitor -lpthread -lmysqlclient

main.o: main.cpp threadPool.h
	g++ -c main.cpp
threadPool.o: threadPool.cpp threadPool.h
	g++ -c threadPool.cpp