powerMonitor: main.o ThreadPool.o
	g++ main.o ThreadPool.o -o powerMonitor -lpthread -lmysqlclient

main.o: main.cpp ThreadPool.h
	g++ -c main.cpp
threadPool.o: ThreadPool.cpp ThreadPool.h
	g++ -c ThreadPool.cpp