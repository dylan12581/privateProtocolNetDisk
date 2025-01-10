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
// 客户端传输协议
typedef struct client_protocol_s{
    int command_flag;
    int parameter_flag;
    int parameter_len;
    int parameter_num;
    char buf[1460];
}protocol_client;
typedef struct server_protocol_s{
    int status;// 状态码：0正常；1异常
    int parameter_flag;//1:携带参数，0:无参数
    int parameter_len;
    int parameter_num;
    char buf[1460];

}protocol_server;
typedef struct file_msg_s{
    int id;
    int userId;
    char path[255];
    int fatherPathId;
    int fileType;
    char fileName[255];
    char md5[50];
    char sha1[50];
    int uploadStatus;
}file_msg_t;

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
/*任务队列相关结构体*/
typedef struct node_s{
    int netFd;
    int flag;// 上传下载标记位
    struct node_s* next;
}node_t;
typedef struct queue_s{
    node_t* head;
    node_t* end;
    int size;
}queue_t;
/*循环数组相关结构体*/
typedef struct ConnectRecord{
    int netFd;
    time_t lastTime;
    struct ConnectRecord* next;
}ConnectRecord;
typedef struct ConnectsStatus{
    // 循环时间队列
    ConnectRecord *table[30];
    int index[1024];
}ConnectsStatus;

/*日志相关枚举*/
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
typedef struct sql_connect_s{
    MYSQL connect;// 数据库连接
    int useFlag;//是否被使用：0空闲，1使用
    int connsIndex;// 数据库连接编号
}sql_connect_t;
/*服务器状态信息(线程池状态)结构体*/
typedef struct pool_s{

    HashMap* map;
    queue_t* taskQueue;

    // 线程池相关
    pthread_t *poolThreadsIds;
    int poolThreadNums;

    pthread_mutex_t threadLock;
    pthread_cond_t cond;

    int exitTag;
    // 数据库相关
    sql_connect_t* conns;
    int connNum;
    pthread_mutex_t connLock;//数据库的锁
    pthread_cond_t connCond;//数据库的条件变量

    // 客户端连接时间的记录
    ConnectsStatus connsStatus;
}pool_t;
/* 数据库连接池相关函数*/
int getConnect(sql_connect_t* sqlConnect,pool_t* pool);
int setConnect(sql_connect_t* sqlConnect,pool_t* pool);


/*哈希表相关函数*/
HashMap* createHashMap();
void insert(HashMap** map,char* key,char* value);
char* get(HashMap* map,char* key);
HashMap* parameterMap();
/*服务器线程池相关函数*/
int initPool(pool_t* pPool);
/*任务队列相关函数*/
int deQueue(queue_t* pQueue);
int enQueue(queue_t* pQueue,int netFd,int flag);
/*TCP连接相关函数*/
int initTcp(int* socketFd,char* ip,char* port);
/*epoll相关函数*/
int epollAdd(int epollFd, int netFd);
int epollRemove(int epollFd, int netFd);
/*子线程相关函数*/
void* threadFunc(void* p);
/*循环数组相关函数*/
int insertConnect(int netFd,ConnectsStatus* connsStatus);
int deleteConnect(int netFd,ConnectsStatus* connsStatus);
int updateConnect(int netFd,ConnectsStatus* connsStatus);
int checkConnect(int epollFd,ConnectsStatus* connsStatus);
/*不同命令*/
int doRequest(int netFd, pool_t* pool);
int loginRequest(int netFd, protocol_client protocol, pool_t* pool);
int registerNameRequest(int netFd, protocol_client protocol, pool_t* pool);
int registerPasswordRequest(int netFd, protocol_client protocol, pool_t* pool);
int mkdirRequest(int netFd, protocol_client protocol, pool_t* pool);
int lsRequest(int netFd, protocol_client protocol, pool_t* pool);
int pathCheckRequest(int netFd, protocol_client protocol, pool_t* pool);
int rmRequest(int netFd, protocol_client protocol, pool_t * pool);
int rmRRequest(int netFd, protocol_client protocol, pool_t * pool);
int getsRequest(int netFd,protocol_client protocol,pool_t* pool);
int putsRequest(int netFd,protocol_client protocol,pool_t* pool);
int dealPuts(int netFd,pool_t* pPool);
int dealGets(int netFd,pool_t* pPool);
/*辅助工具函数*/
int getParameterMap(protocol_client protocol,HashMap* map);
int stringCat(char* buf,char* key,char* value);
/* 文件上传相关 */
int uploadFile(int netFd,int fileFd);
int uploadFileBreakPoint(int netFd,FILE* file);
int sendFile(int netFd,int fileFd);



