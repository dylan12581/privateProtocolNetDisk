#include "head.h"
/*Usage :*/
int insertConnect(int netFd,ConnectsStatus* connsStatus){
    // 初始化信息
    ConnectRecord* connRecord = (ConnectRecord* ) calloc(1,sizeof(ConnectRecord));
    time(&connRecord->lastTime);
    connRecord->netFd = netFd;

    // 计算时间的余数
    int timeIndex = connRecord->lastTime % 30;
    // 记录余数锁代表的下标位置
    connsStatus->index[netFd] = timeIndex;

    if(connsStatus->table[timeIndex] == NULL){
        connsStatus->table[timeIndex] = connRecord;
    }else{
        connRecord->next = connsStatus->table[timeIndex];
        connsStatus->table[timeIndex] = connRecord;
    }
    return 0;
}
int deleteConnect(int netFd,ConnectsStatus* connsStatus){
    int oldIndex = connsStatus->index[netFd];
    ConnectRecord * conn;// 为了释放内存
    ConnectRecord * temp = connsStatus->table[oldIndex];
    if(temp->netFd == netFd){
        conn = temp;
        connsStatus->table[oldIndex] = temp->next;
    }else{
        // 不是头位置，遍历找
        while(temp->next != NULL){
            if(temp->next->netFd != netFd){
                temp = temp->next;
            }else{
                conn = temp->next;
                temp->next = temp->next->next;
                break;
            }
        }
    }
    connsStatus->index[netFd] = 0;//
    free(conn);
    return 0;
}
int updateConnect(int netFd,ConnectsStatus* connsStatus){

    int oldIndex = connsStatus->index[netFd];
    // 先移除记录
    ConnectRecord* update;
    ConnectRecord* temp = connsStatus->table[oldIndex];
    if(temp->netFd == netFd){
        update = temp;
        connsStatus->table[oldIndex] = temp->next;
    }else{
        while(temp->next != NULL){
            if(temp->next->netFd != netFd){
                temp = temp->next;
            }else{
                update = temp->next;
                temp->next = temp->next->next;
                break;
            }
        }
    }
    // 再插入记录
    time(&update->lastTime);
    // 计算时间余数
    int newIndex = update->lastTime % 30;

    connsStatus->index[netFd] = newIndex;

    if(connsStatus->table[newIndex] == NULL){
        connsStatus->table[newIndex] = update;
    }else{
        update->next = connsStatus->table[newIndex];
        connsStatus->table[newIndex] = update;
    }
    return 0;
}

int checkConnect(int epollFd,ConnectsStatus* connsStatus){
    time_t currTime;
    time(&currTime);

    int index = currTime % 30;
    ConnectRecord * conn;
    ConnectRecord* temp  = connsStatus->table[index];

    // 先处理非头结点
    while(temp != NULL && temp->next != NULL){
        if(currTime - temp->next->lastTime >= 30){
            conn = temp->next;
            temp->next = temp->next->next;

            epollRemove(epollFd,conn->netFd);
            close(conn->netFd);
            connsStatus->index[conn->netFd] = 0;
            free(conn);
        }else{
            temp = temp->next;
        }
    }
    // 对头结点进行判断
    if(connsStatus->table[index] != NULL && currTime - connsStatus->table[index]->lastTime >= 30){
        // 头部超时 
        conn = connsStatus->table[index];
        connsStatus->table[index] = conn->next;
        epollRemove(epollFd,conn->netFd);
        close(conn->netFd);
        connsStatus->index[conn->netFd] = 0;
        free(conn);

        printf("超时踢出\n");
        LOG(LOG_TYPE_INFO, "客户端被超时剔除");
    }
    return 0;
}
