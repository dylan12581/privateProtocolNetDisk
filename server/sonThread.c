#include "head.h"
// 子线程的入口函数，子线程只负责上传下载文件
void* threadFunc(void* p){
    pool_t* pPool = (pool_t*)p;
    while(1){
        

        pthread_mutex_lock(&pPool->threadLock);
        // 去任务队列中获取任务
        while(pPool->taskQueue->size <= 0 && pPool->exitTag == 0){
            pthread_cond_wait(&pPool->cond,&pPool->threadLock);
        }
        // 检测是否要退出
        if(pPool->exitTag == 1){
            pthread_mutex_unlock(&pPool->threadLock);
            return 0;
        }
        int netFd = pPool->taskQueue->head->netFd;
        int flag = pPool->taskQueue->head->flag;
        deQueue(pPool->taskQueue);
        pthread_mutex_unlock(&pPool->threadLock);

        if(flag == 0){
            dealPuts(netFd,pPool);
        }else{
            dealGets(netFd,pPool);
        }
        // 释放资源
        close(netFd);
    }
}
