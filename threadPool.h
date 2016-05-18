//@file: threadPool.h
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
class threadPool
{
public:
	threadPool(int threadNum);
	//~threadPool();
	void addWork(int fd);
private:
 	static void *threadFunc(void *ptr);
	std::vector<pthread_t> threadList_;
	std::deque<int> workQueue_;
	pthread_mutex_t queueLock_;
	pthread_mutex_t dbLock_;
	pthread_cond_t queueReady_;
	int threadPoolSize_;
	MYSQL mysql_;
};
#endif //_THREADPOOL_H_