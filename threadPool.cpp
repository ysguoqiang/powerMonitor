//@file: threadPool.h
//@brief 
//@realization for thread pool
//author: Ricky Fu
//@date: 2016-5-12 
#include "threadPool.h"
void* threadPool::threadFunc(void *ptr)
{
    int fd;
    int n;
    char readLine[100];
    pthread_t tid = pthread_self();
    threadPool *pthreadpool = (threadPool *)ptr;
    for( ; ; )
    {
        std::cout << "tid =" << (unsigned long)tid << " is running" << std::endl;
        n = 0;
        pthread_mutex_lock(&pthreadpool->queueLock_);
        while(pthreadpool->workQueue_.empty())
            pthread_cond_wait(&pthreadpool->queueReady_, &pthreadpool->queueLock_);
        fd = pthreadpool->workQueue_.back();
        pthreadpool->workQueue_.pop_front();
        pthread_mutex_unlock(&pthreadpool->queueLock_);
        for( ; ; )
        {
            int ret;
            ret = read(fd, readLine + n, 100);
            if(ret < 0 && ret != EAGAIN)
                break;
            else if(ret == EAGAIN)
                continue;
            n += ret;
        }
        close(fd);
        pthread_mutex_lock(&pthreadpool->dbLock_);
        mysql_real_connect(&pthreadpool->mysql_, "localhost", "root", "", "powerMonitor", 0, NULL, 0);
        std::string sql = "insert into runningInfo(onOff, power, voltage, current) values(1, 101, 220, 2);";
        mysql_query(&pthreadpool->mysql_,sql.c_str());
        mysql_close(&pthreadpool->mysql_);
        pthread_mutex_unlock(&pthreadpool->dbLock_);
    }
}

threadPool::threadPool(int threadNum): threadPoolSize_(threadNum)
{    
    pthread_mutex_init(&queueLock_, NULL);
    pthread_mutex_init(&dbLock_, NULL);
    pthread_cond_init(&queueReady_, NULL);
    //create thread pool
    mysql_init(&mysql_);
    std::cout << "create " << threadNum << " threads" << std::endl;
    for(int i = 0; i < threadNum; i++) 
    {
        pthread_t tid;
        pthread_create(&tid , NULL, threadFunc, (void *)this);
          //  std::cout << "failed to create thread" << std::endl; 
        threadList_.push_back(tid);
    }
}



void threadPool::addWork(int fd)
{
    pthread_mutex_lock(&queueLock_);
    workQueue_.push_back(fd);
    pthread_mutex_unlock(&queueLock_);
    pthread_cond_signal(&queueReady_);
}