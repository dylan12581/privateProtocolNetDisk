#include "head.h"

int distributeCommand(run_status_t* runStatus){
    printf("%s",runStatus->user_name);
    printf("@:");
    printf("%s",runStatus->path);
    printf("$");
    fflush(stdout);
    
    char buf[1024] = {0};
    ssize_t res = read(STDIN_FILENO,buf,sizeof(buf) - 1);

    //处理命令最后的换行
    if(res > 0){
        buf[res - 1] = '\0';
    }
    //处理命令最前的空格
    int start = 0;
    while(buf[start] == ' '){
        start++;
    }
    //处理命令最后的空格
    int end = strlen(buf) - 1;
    while(end > start && buf[end] == ' '){
        end--;
    }
    buf[end + 1] = 0;
    // 移除字符串以去除前导空格
    for(int i = start,j = 0;i <= end;i++,j++){
        buf[j] = buf[i];
    }
    buf[end - start + 1] = '\0';
    // 处理命令和参数之间多余的空格
    int writeIndex = 0;
    for(int i = 0;buf[i] !='\0';i++){
        if(buf[i] == ' '&& buf[writeIndex-1] == ' ' ){
            continue;
        }
        buf[writeIndex++] = buf[i];
    }
    buf[writeIndex] = 0;
    printf("Processed command:\"%s\"\n",buf);


    if(strncmp(buf,"ls",2) == 0){
        lsCommand(buf,runStatus);
    }else if(strncmp(buf,"cd",2) == 0){
        cdCommand(buf,runStatus);
    }else if(strncmp(buf,"pwd",3) == 0){
        pwdCommand(buf,runStatus);
    }else if(strncmp(buf,"rm",2) == 0){
        rmCommand(buf,runStatus);
    }else if(strncmp(buf,"mkdir",5) == 0){
        mkdirCommand(buf,runStatus);
    }else if(strncmp(buf,"puts",4) == 0){
        putsCommand(buf,runStatus);
    }else if(strncmp(buf,"gets",4) == 0){
        getsCommand(buf,runStatus);
    }
    else{
        printf("非法输入，请重新输入\n");
    }
    return 0;
}
