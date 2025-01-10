#include "head.h"

int epollAdd(int epollFd, int netFd){

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = netFd;

    epoll_ctl(epollFd, EPOLL_CTL_ADD, netFd, &event);

    return 0;
}

int epollRemove(int epollFd, int netFd){

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = netFd;

    epoll_ctl(epollFd, EPOLL_CTL_DEL, netFd, &event);

    return 0;
}

