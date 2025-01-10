#include "head.h"

int putsCommand(char *buf, run_status_t *runstatus){

    // puts命令后面没有跟内容
    if(buf[4] == 0){
        printf("请输入要上传文件的名子 \n");
        return 0;
    }
    // 判断: putsr, putsx....
    if(buf[4] != ' '){
        printf("非法命令 \n");
        return 0;
    }
    char fileName[1024] = {0};
    for(int i=0; i<strlen(buf); i++ ){
        if(buf[i+5] == 0){
            break;
        }
        fileName[i] = buf[i+5];
    }
    // 构建交给子线程上传任务的数据
    PutsData data;
    memset(&data,0,sizeof(data));
    memcpy(data.token,runstatus->token,strlen(runstatus->token));
    memcpy(data.path,runstatus->path,strlen(runstatus->path));
    memcpy(data.fileName,fileName,strlen(fileName));
    char* ip = get(runstatus->map,"ip");
    memcpy(data.ip,ip,strlen(ip));
    char* port = get(runstatus->map,"port");
    memcpy(data.port,port,strlen(port));

    // 创建子线程
    pthread_t sonId;
    pthread_create(&sonId,NULL,threadFunc,&data);

    usleep(1000);
    return 0;
}
