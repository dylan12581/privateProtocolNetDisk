#include "head.h"

int getsRequest(int netFd,protocol_client protocol,pool_t* pool){
    pthread_mutex_lock(&pool->threadLock);
    enQueue(pool->taskQueue,netFd,1);
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->threadLock);
    return 0;
}
int putsRequest(int netFd,protocol_client protocol,pool_t* pool){

    pthread_mutex_lock(&pool->threadLock);
    enQueue(pool->taskQueue,netFd,0);
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->threadLock);
    return 0;
}

int otherRequest(int netFd,protocol_client protocol,pool_t* pool){
    return 0;
}
int doRequest(int netFd,pool_t* pool){

    int msgLen = 0;
    int recv_res = recv(netFd,&msgLen,sizeof(int),MSG_WAITALL);
    if(recv_res == 0 ){
        return -1;
    }

    protocol_client protocol;
    bzero(&protocol,sizeof(protocol_client));
    recv_res = recv(netFd,&protocol,msgLen,MSG_WAITALL);
    if(recv_res == 0){
        return -1;
    }

    int commandTag = protocol.command_flag;

    int res = 0;
    switch(commandTag){
    case LOGIN:
        res = loginRequest(netFd,protocol,pool);
        return res;
    case REGISTER_NAME:
        res = registerNameRequest(netFd, protocol, pool);
        return res;
    case REGISTER_PASSWORD:
        res = registerPasswordRequest(netFd, protocol, pool);
        return res;
    case LS:
        res = lsRequest(netFd, protocol, pool);
        return res;
    case PATHCHECK:
        res = pathCheckRequest(netFd, protocol, pool);
        return res;
    case RM:
        res = rmRequest(netFd, protocol, pool);
        return res;
    case RMR:
        res = rmRRequest(netFd, protocol, pool);
        return res;
    case MKDIR:
        res = mkdirRequest(netFd, protocol, pool);
        return res;
    case PUTS_SYNC:
        // 调用子进程处理文件上传
        putsRequest(netFd, protocol, pool);
        // 返回命令状态
        return PUTS_SYNC;
    case GETS_SYNC:
        getsRequest(netFd, protocol, pool);
        return GETS_SYNC;
    default:
        res = otherRequest(netFd, protocol, pool);
        return res;
    }
}
