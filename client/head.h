#include <my_header.h>

enum{
    LOGIN = 100,
    REGISTER_NAME = 101,
    REGISTER_PASSWORD = 102,
    LS = 110,
    CD = 120,
    PATHCHECK = 121,
    PWD = 130,
    RM = 140,
    RMR = 141,
    MKDIR = 150,
    PUTS = 200,
    PUTS_SYNC = 201,
    GETS = 300,
    GETS_SYNC = 301,
};
typedef struct {
    char path[1000];
    char token[1024];
    char fileName[1024];
    char ip[100];
    char port[100];
    int flag;//上传文件，下载文件
}PutsData;
// 客户端传输协议格式
typedef struct client_protocol_s{
    int command_flag;
    int parameter_flag;
    int parameter_len;
    int parameter_num;
    char buf[1460];
}protocol_client;
// 服务端传输协议响应格式
typedef struct server_protocol_s{
    int status;// 状态码：0正常；1异常
    int parameter_flag;//1:携带参数，0:无参数
    int parameter_len;
    int parameter_num;
    char buf[1460];
}protocol_server;

/*哈希表相关结构体*/
typedef struct Node{
    char* key;
    char* value;
    Node* next;
}Node;
typedef struct HashMap{
    Node** table;
    int capacity;
    int size;
}HashMap;
typedef struct run_status_s{
    int login_status;
    char user_name[100];
    int token_len;
    char token[1024];
    int path_len;
    char path[1024];
    int net_fd;
    HashMap* map;
}run_status_t;


enum {
    LOG_TYPE_ERROR = 1,
    LOG_TYPE_WARNING = 2,
    LOG_TYPE_INFO = 3,
};
// 日志宏函数
#define LOG(flag,msg){\
    HashMap *map = parameterMap();\
    char* base_log_path = get(map, "log_path");\
    char* log_level = get(map, "log_level"); \
    if(flag == LOG_TYPE_ERROR && (strncmp(log_level, "error", 5)== 0 ||strncmp(log_level, "warning", 7)== 0 || strncmp(log_level, "info", 4) == 0) ){ \
        char logName[200] = {0}; \
        sprintf(logName, "%s/log_error.log", base_log_path); \
        int file_fd = open(logName, O_RDWR |O_CREAT|O_APPEND, 0600);  \
        char buf[1024] = {0}; \
        time_t now = time(NULL); \
        struct tm *local_time = localtime(&now); \
        strftime(buf, sizeof(buf), "[ %Y-%m-%d %H:%M:%S ]", local_time); \
        sprintf(buf + strlen(buf), " [pid : %d]", getpid()) ; \
        sprintf(buf + strlen(buf), " [ ERROR ]" ); \
        sprintf(buf + strlen(buf), " [%s :: %s :: %d]: %s \n", __FILE__, __FUNCTION__, __LINE__, msg) ; \
        write(file_fd, buf, strlen(buf));\
        close(file_fd);\
        bzero(logName, 200);\
        sprintf(logName, "%s/log_all.log", base_log_path); \
        file_fd = open(logName, O_RDWR |O_CREAT|O_APPEND, 0600);  \
        write(file_fd, buf, strlen(buf));\
        close(file_fd);\
    }else if(flag == LOG_TYPE_WARNING && (strncmp(log_level, "warning", 7)==0 || strncmp(log_level, "info", 4)==0)){\
        char logName[200] = {0}; \
        sprintf(logName, "%s/log_waring.log", base_log_path); \
        int file_fd = open(logName, O_RDWR |O_CREAT|O_APPEND, 0600);  \
        char buf[1024] = {0}; \
        time_t now = time(NULL); \
        struct tm *local_time = localtime(&now); \
        strftime(buf, sizeof(buf), "[ %Y-%m-%d %H:%M:%S ]", local_time); \
        sprintf(buf + strlen(buf), " [pid : %d]", getpid()) ; \
        sprintf(buf + strlen(buf), " [ WARNING ]") ; \
        sprintf(buf + strlen(buf), " [%s :: %s :: %d]: %s \n", __FILE__, __FUNCTION__, __LINE__, msg) ; \
        write(file_fd, buf, strlen(buf));\
        close(file_fd);\
        bzero(logName, 200);\
        sprintf(logName, "%s/log_all.log", base_log_path); \
        file_fd = open(logName, O_RDWR |O_CREAT|O_APPEND, 0600);  \
        write(file_fd, buf, strlen(buf));\
        close(file_fd);\
    }else if(flag == LOG_TYPE_INFO && (strncmp(log_level, "info", 4)==0)){\
        char logName[200] = {0}; \
        sprintf(logName, "%s/log_info.log", base_log_path); \
        int file_fd = open(logName, O_RDWR |O_CREAT|O_APPEND, 0600);  \
        char buf[1024] = {0}; \
        time_t now = time(NULL); \
        struct tm *local_time = localtime(&now); \
        strftime(buf, sizeof(buf), "[ %Y-%m-%d %H:%M:%S ]", local_time); \
        sprintf(buf + strlen(buf), " [pid : %d]", getpid()) ; \
        sprintf(buf + strlen(buf), " [ INFO ]") ; \
        sprintf(buf + strlen(buf), " [%s :: %s :: %d]: %s \n", __FILE__, __FUNCTION__, __LINE__, msg) ; \
        write(file_fd, buf, strlen(buf));\
        close(file_fd);\
        bzero(logName, 200);\
        sprintf(logName, "%s/log_all.log", base_log_path); \
        file_fd = open(logName, O_RDWR |O_CREAT|O_APPEND, 0600);  \
        write(file_fd, buf, strlen(buf));\
        close(file_fd);\
    } \
}


/*哈希表相关函数*/
HashMap* createHashMap();
void insert(HashMap** map,char* key,char* value);
char* get(HashMap* map,char* key);
HashMap* parameterMap();
/*TCP连接相关函数*/
int initTcp(int* socketFd,char* ip,char* port);
/*epoll相关函数*/
int epollAdd(int epollFd, int netFd);
int epollRemove(int epollFd, int netFd);
/*子线程相关函数*/
void* threadFunc(void* p);
/*不同命令*/
int distributeLoginOrRegister(run_status_t* runStatus);
int distributeCommand(run_status_t* runStatus);
int doLogin(run_status_t *runstatus);
int doRegister(run_status_t* runstatus);
int mkdirCommand(char *buf, run_status_t *runstatus);
int lsCommand(char *buf, run_status_t *runstatus);
int cdCommand(char *buf, run_status_t *runstatus);
int pwdCommand(char *buf, run_status_t *runstatus);
int rmCommand(char *buf, run_status_t *runstatus);
int putsCommand(char *buf, run_status_t *runstatus);
int getsCommand(char *buf, run_status_t *runstatus);

/*辅助工具函数*/
int getParameterMap(protocol_client protocol,HashMap* map);
int stringCat(char* buf,char* key,char* value);
/* 文件上传相关 */
int uploadFile(int netFd,int fileFd);
int uploadFileBreakPoint(int netFd,FILE* file);
int sendFile(int netFd,int fileFd);



