#include "head.h"

int dealPuts(int netFd,pool_t* pool){

     int msgLen = 0;
     ssize_t sret = recv(netFd,&msgLen,sizeof(msgLen),MSG_WAITALL);

     protocol_client protocol;
     bzero(&protocol,sizeof(protocol_client));
     recv(netFd,&protocol,msgLen,MSG_WAITALL);

    /*解析参数*/
    HashMap *map = createHashMap();
    getParameterMap(protocol,map);

    char* token = get(map,"token");
    char* path = get(map,"path");
    char* fileName = get(map,"fileName");

    //根据token 获取用户id
    int userId = 0;
    userId = getUserId(token);
    if(userId == -1){
        return 0;
    }

    //进行sql查询
    sql_connect_t sqlConnect;
    bzero(&sqlConnect,sizeof(sql_connect_t));
    getConnect(&sqlConnect,pool);
    char sqlBuf[1024] = {0};
    sprintf(sqlBuf,
            "select * from file where delete_flag = 0 and file_name = '%s' and father_path_id = "
            "(select id from file where delete_flag = 0 and user_id = %d and path = '%s')",
            fileName,userId,path);
    mysql_query(&sqlConnect.connect,sqlBuf);
    MYSQL_RES* result = mysql_store_result(&sqlConnect.connect);
    my_ulonglong rowNum = mysql_num_rows(result);

    // 构建回复的报文
    protocol_server protocolRes;
    memset(&protocolRes,0,sizeof(protocol_server));
    int sendLen = sizeof(int)*4 + protocolRes.parameter_len;

    if(rowNum > 0){
        // 原路径下有这个文件
        MYSQL_ROW row = mysql_fetch_row(result);
        // 有这个文件，获取hash值
        char sha1[100] = {0};
        memcpy(sha1,row[7],sizeof(sha1));
        send(netFd,&sendLen,sizeof(int),MSG_NOSIGNAL);
        send(netFd,&protocolRes,sendLen,MSG_NOSIGNAL);

        // 打开文件
        char hashFilePathName[1024] = {0};
        char* basePath = get(pool->map,"base_path");
        sprintf(hashFilePathName,"%s%s",basePath,sha1);

        int fileFd = open(hashFilePathName,O_RDWR);
        sendFile(netFd,fileFd);
        close(fileFd);
    }else{
        // 当前路径下不存在该文件
        protocolRes.status = -1;
        send(netFd,&sendLen,sizeof(int),MSG_NOSIGNAL);
        send(netFd,&protocolRes,sendLen,MSG_NOSIGNAL);
    }
    mysql_free_result(result);
    
    // 使用完毕之后，将数据库连接还回去
    setConnect(&sqlConnect,pool);
    return 0;
}
