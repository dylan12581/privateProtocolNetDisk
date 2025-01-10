#include "head.h"

int getConnect(sql_connect_t* sqlConnect,pool_t* pool){
    sqlConnect->connsIndex = -1;
    while(1){
        pthread_mutex_lock(&pool->connLock);
        for(int i = 0;i < pool->connNum;++i){
            if(pool->conns[i].useFlag == 0){
                sqlConnect->useFlag = 1;
                sqlConnect->connect = pool->conns[i].connect;
                sqlConnect->connsIndex = i;
            }
        }
        if(sqlConnect->connsIndex == -1){
            LOG(LOG_TYPE_WARNING,"数据库连接池连接不够！");
            pthread_cond_wait(&pool->connCond,&pool->connLock);
            pthread_mutex_unlock(&pool->connLock);
        }else{
            pthread_mutex_unlock(&pool->connLock);
            break;
        }
    }
    return 0;
}
int setConnect(sql_connect_t* sqlConnect,pool_t* pool){
    pthread_mutex_lock(&pool->connLock);
    pool->conns[sqlConnect->connsIndex].useFlag = 0;
    pool->conns[sqlConnect->connsIndex].connsIndex = -1;
    pthread_cond_broadcast(&pool->connCond);
    pthread_mutex_unlock(&pool->connLock);
    return 0;
}
