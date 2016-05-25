//@file: ThreadPool.h
//@brief 
//@realization for thread pool
//author: Ricky Fu
//@date: 2016-5-12 
#include "ThreadPool.h"
#include <sstream>
void* ThreadPool::threadWriteFunc(void *ptr)
{
    int fd;
    int n;
    char readBuffer[100];
    pthread_t tid = pthread_self();
    ThreadPool *pthreadpool = (ThreadPool *)ptr;
    for( ; ; )
    {
        std::cout << "tid =" << (unsigned long)tid << " is running" << std::endl;
        n = 0;
        pthread_mutex_lock(&pthreadpool->writeWorkQueueLock_);
        while(pthreadpool->writeWorkQueue_.empty())
            pthread_cond_wait(&pthreadpool->writeWorkQueueReady_, &pthreadpool->writeWorkQueueLock_);
        fd = pthreadpool->writeWorkQueue_.front();
        pthreadpool->writeWorkQueue_.pop_front();
        pthread_mutex_unlock(&pthreadpool->writeWorkQueueLock_);
        for( ; ; )
        {
            int ret;
            ret = read(fd, readBuffer + n, 100);
            if(ret < 0 && ret != EAGAIN)
                break;
            else if(ret == EAGAIN)
                continue;
            n += ret;
    	    std::cout << "n is " << n << std::endl;	
    	    std::cout << "readBuffer: " << (int) readBuffer[6] << (int) readBuffer[7] << (int) readBuffer[8] << (int) readBuffer[9] << (int) readBuffer[10] << (int) readBuffer[11] << (int) readBuffer[12] << (int) readBuffer[13] << std::endl;
    	    if(n >= 17)
    		  break;
        }
        close(fd);
    	if(n != 17)
    	{
    	    std::cout << "close fd = " << fd <<std::endl;
     	    std::cout << "n = " << n << std::endl;
    	    continue;
    	}
        pthread_mutex_lock(&pthreadpool->dbLock_);
	    int onOff = readBuffer[6];
        std::ostringstream oss;
        std::string onOffStr, powerStr, voltageStr, currentStr;
        oss << onOff;
        onOffStr = oss.str();
        oss.str("");
        oss << (int) readBuffer[7];
        voltageStr = oss.str();     
        oss.str("");
        oss << (int) readBuffer[8];
        voltageStr = voltageStr + '.' + oss.str();
        oss.str("");
        oss << (int) readBuffer[9];
        currentStr = oss.str();
        oss.str("");
        oss << (int) readBuffer[10];
        currentStr = currentStr + '.' + oss.str();
        oss.str("");
        oss << (int) readBuffer[12];
        powerStr = oss.str();
        oss.str("");
        oss << (int) readBuffer[13];
        powerStr = powerStr + '.' + oss.str();
        oss.str("");
        mysql_real_connect(&pthreadpool->mysql_, "localhost", "root", "", "powerMonitor", 0, NULL, 0);
        // std::string sql = "insert into runningInfo(onOff, power, voltage, current) values(1, 101, 220, 2);";
        std::string sql = "insert into runningInfo(onOff, power, voltage, current) values(" + onOffStr + ',' + powerStr + ',' + voltageStr + ',' + currentStr + ");";
        std::cout << "write to mysql = " << sql <<std::endl;
	    mysql_query(&pthreadpool->mysql_,sql.c_str());
        mysql_close(&pthreadpool->mysql_);
        pthread_mutex_unlock(&pthreadpool->dbLock_);
    }
}

ThreadPool::ThreadPool(int threadNum): ThreadPoolSize_(threadNum)
{    
    pthread_mutex_init(&writeWorkQueueLock_, NULL);
    pthread_mutex_init(&dbLock_, NULL);
    pthread_cond_init(&writeWorkQueueReady_, NULL);
    //create thread pool
    mysql_init(&mysql_);
    std::cout << "create " << threadNum << " threads" << std::endl;
    for(int i = 0; i < threadNum; i++) 
    {
        pthread_t tid;
        pthread_create(&tid , NULL, threadWriteFunc, (void *)this);
          //  std::cout << "failed to create thread" << std::endl; 
        threadList_.push_back(tid);
    }
}



void ThreadPool::addWriteWork(int fd)
{
    pthread_mutex_lock(&writeWorkQueueLock_);
    writeWorkQueue_.push_back(fd);
    pthread_mutex_unlock(&writeWorkQueueLock_);
    pthread_cond_signal(&writeWorkQueueReady_);
}
