#include "head.h"
/*Usage :*/

int pipeFd[2];// 全局类型的匿名管道
void func(int num){// 注册二号信号，用于主线程通知子线程退出
    write(pipeFd[1],"1",1);
}
int main(int argc,char* argv[])
{
    if(fork() != 0){// 主线程逻辑
        signal(SIGINT,func);
        wait(NULL);

        exit(0);
    }
    // 子线程逻辑
    setpgid(0,0);

    pool_t pool;
    initPool(&pool);

    int socketFd;
    initTcp(&socketFd,get(pool.map,"ip"),get(pool.map,"port"));

    int epollFd = epoll_create(1);
    epollAdd(epollFd,socketFd);
    epollAdd(epollFd,pipeFd[0]);

    while(1){
        struct epoll_event ready[10];
        int epollNum = epoll_wait(epollFd,ready,10,1000);// 1s就绪一次

        for(int i = 0; i < epollNum ; ++i){
            // 主线程通知子线程有序退出
            if(ready[i].data.fd == pipeFd[0]){
                char buf[60] = {0};
                read(pipeFd[0],buf,sizeof(buf));

                pthread_mutex_lock(&pool.threadLock);
                pool.exitTag = 1;
                pthread_cond_broadcast(&pool.cond);
                pthread_mutex_unlock(&pool.threadLock);

                for(int k = 0;k < pool.poolThreadNums;k++){
                    pthread_join(pool.poolThreadsIds[k],NULL);
                }
                exit(0);
            }else if (ready[i].data.fd == socketFd){
                int netFd = accept(socketFd,NULL,NULL);
                epollAdd(epollFd,netFd);
                // 加入循环的时间队列
                insertConnect(netFd,&pool.connsStatus);
                LOG(LOG_TYPE_INFO,"新连接进来");
            }else{
                // 客户端的请求进来
                int res = doRequest(ready[i].data.fd,&pool);
                if(res == PUTS_SYNC || res == GETS_SYNC){
                    // 为什么没有必要继续监听这个连接了？？？不是很清楚
                    // TODO:如果是文件上传，会交给子线程处理，没有必要在主线程中继续监听这个连接了
                    epollRemove(epollFd,ready[i].data.fd);
                    // 并且将此连接移除当前的循环队列
                    deleteConnect(ready[i].data.fd,&pool.connsStatus);
                    continue;
                }else if (res == -1){
                    LOG(LOG_TYPE_INFO,"客户端连接断开");
                    epollRemove(epollFd,ready[i].data.fd);
                    close(ready[i].data.fd);
                    // 并且将此连接移除当前的循环队列
                    deleteConnect(ready[i].data.fd,&pool.connsStatus);
                    continue;
                }
                // 在循环队列中更新
                updateConnect(ready[i].data.fd,&pool.connsStatus);
            }
        }
        //每一秒epoll就绪一次都会进行判断超时踢出
        checkConnect(epollFd,&pool.connsStatus);
    }
    return 0;
}

