#include "head.h"

int uploadFile(int netFd,int fileFd){
    while(1){
        char buf[4096] = {0};
        ssize_t ret =  recv(netFd,buf,sizeof(buf),MSG_WAITALL);
        if(ret == 0){
            LOG(LOG_TYPE_INFO,"客户端上传文件成功！");
            break;
        }
        write(fileFd,buf,ret);
    }
    return 0;
}
/* 断点续传 */
int uploadFileBreakPoint(int netFd,FILE* file){

    while(1){
        char buf[4096] = {0};
        ssize_t ret =  recv(netFd,buf,sizeof(buf),MSG_WAITALL);
        if(ret == 0){
            LOG(LOG_TYPE_INFO,"客户端断点续传文件成功！");
            break;
        }
        fwrite(buf,1,ret,file);
    }
    return 0;
}

int sendFile(int netFd,int fileFd){
    //while(1){
    //    char buff[4096] = {0};
    //    ssize_t res = recv(fileFd,buff,sizeof(buff),MSG_WAITALL);
    //    if(res == 0){
    //        break;
    //    }
    //    send(netFd,buff,res,MSG_NOSIGNAL);
    //}
    struct stat st;
    fstat(fileFd,&st);
    sendfile(netFd,fileFd,0,st.st_size);
    return 0;
}
