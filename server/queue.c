#include "head.h"

int deQueue(queue_t* pQueue){
    if(pQueue->size == 0){
        return -1;
    }
    node_t* p = pQueue->head;
    if(pQueue->size == 1){
        pQueue->head = NULL;
        pQueue->end = NULL;
    }else{
        pQueue->head = pQueue->head->next;
    }
    pQueue->size--;
    free(p);
    return 0;
}
            
int enQueue(queue_t* pQueue,int netFd,int flag){
    node_t* pNode = (node_t*)calloc(1,sizeof(node_t));
    pNode->netFd = netFd;
    pNode->flag = flag;

    if(pQueue->size == 0){
        pQueue->head = pNode;
        pQueue->end = pNode;
    }else{
        pQueue->end->next= pNode;
        pQueue->end = pNode;
    }
    pQueue->size++;
    return 0;
}
    
