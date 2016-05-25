//@file: ThreadPool.h
//@brief 
//@declaration for thread pool
//author: Ricky Fu
//@date: 2016-5-12 
#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_
#include <pthread.h>
#include <vector>
#include <deque>
#include <mysql/mysql.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <iostream>
class ThreadPool
{
public:
	ThreadPool(int threadNum);
	//~ThreadPool();
	void addWriteWork(int fd);
	//void addReadWork(int fd);
private:
 	static void *threadWriteFunc(void *ptr);
 	//static void *threadReadFunc(void *ptr);
	std::vector<pthread_t> threadList_;
	std::deque<int> writeWorkQueue_;
	//std::deque<int> readWorkQueue_;
	pthread_mutex_t writeWorkQueueLock_;
	pthread_mutex_t dbLock_;
	pthread_cond_t writeWorkQueueReady_;
	int ThreadPoolSize_;
	MYSQL mysql_;
};
#endif //_THREADPOOL_H_