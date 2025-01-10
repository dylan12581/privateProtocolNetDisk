#include "head.h"

int initPool(pool_t* pPool){
    // 初始化循环队列
    bzero(&pPool->connsStatus,sizeof(ConnectsStatus));

    // 根据配置文件内容建立hashMap
    pPool->map = parameterMap();
    pPool->poolThreadNums = atoi(get(pPool->map,"thread_num"));

    // TODO:初始化任务队列
    bzero(pPool->taskQueue,sizeof(queue_t));
     
    // 初始化线程池锁和条件变量
    pthread_mutex_init(&pPool->threadLock,NULL);
    pthread_cond_init(&pPool->cond,NULL);
    // 初始化退出标记位
    pPool->exitTag = 0;
    // 初始化线程池
    pPool->poolThreadsIds = (pthread_t*)calloc(pPool->poolThreadNums,sizeof(pthread_t));
    for(int i = 0;i < pPool->poolThreadNums;++i){
        // 创建子线程
        pthread_create(&pPool->poolThreadsIds[i],NULL,threadFunc,pPool);
    }
    // TODO:初始化数据连接池的内容
    pthread_mutex_init(&pPool->connLock,NULL);
    pthread_cond_init(&pPool->connCond,NULL);
    pPool->connNum = atoi(get(pPool->map,"conn_num"));
    pPool->conns = (sql_connect_t*) calloc(pPool->connNum,sizeof(sql_connect_t));
    for(int i = 0;i < pPool->connNum;++i){
        pthread_mutex_lock(&pPool->connLock);
        mysql_init(&pPool->conns[i].connect);
        mysql_real_connect(&pPool->conns[i].connect,
                           get(pPool->map,"mysql_location"),
                           get(pPool->map,"mysql_user"),
                           get(pPool->map,"mysql_password"),
                           get(pPool->map,"mysql_db"),
                           0,
                           NULL,
                           0);
        pPool->conns[i].useFlag = 0;
        pPool->conns[i].connsIndex = i;
        pthread_mutex_unlock(&pPool->connLock);
    }
    return 0;
}
