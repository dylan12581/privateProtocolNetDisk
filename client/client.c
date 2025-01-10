#include "head.h"
/*Usage :*/

int main(int argc,char* argv[])
{
    run_status_t runStatus;
    memset(&runStatus,0,sizeof(run_status_t));
    runStatus.map = parameterMap();
    strcpy(runStatus.path,"/");

    initTcp(&runStatus.net_fd,get(runStatus.map,"ip"),get(runStatus.map,"port"));

    while(1){
        if(runStatus.login_status == 0){
            distributeLoginOrRegister(&runStatus);
        }else{
            distributeCommand(&runStatus);
        }
    }
    return 0;
}

