#include "head.h"


int distributeLoginOrRegister(run_status_t* runStatus){
    printf("请选择 (Y)登陆, (R)注册, (E)退出: ");
    fflush(stdout);

    char choose[10];
    scanf("%s",choose);

    switch(choose[0]){
    case 'Y':
        doLogin(runStatus);
        break;
    case 'R':
        doRegister(runStatus);
        break;
    case 'E':
        exit(0);
        break;
    default:
        printf("错误字符,请重新输入 \n");
    }
    return 0;
}
int doLogin(run_status_t* runStatus){
    return 1;
}
int doRegister(run_status_t* runStatus){
    return 1;
}
