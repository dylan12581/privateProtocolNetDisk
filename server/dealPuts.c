#include "head.h"
int dealPuts(int netFd,pool_t* pool){
    /*接收客户端上传的文件信息*/
    time_t firstTime;
    time(&firstTime);

    char* p = (char*)malloc(5);
    int len = 0;
    while(len < 4){
        ssize_t sret = recv(netFd,p + len,4 - len,MSG_DONTWAIT);
        if(sret == 0){
            return 0;
        }
        if(sret == -1){
            time_t nowTime;
            time(&nowTime);
            if(nowTime - firstTime > 5){
                return 0;
            }
            continue;
        }
        len += sret;
    }
    int* msgLen = (int*)p;
    protocol_client protocol;
    bzero(&protocol,sizeof(protocol_client));
    recv(netFd,&protocol,*msgLen,MSG_WAITALL);
    /*解析参数*/
    HashMap *map = createHashMap();
    getParameterMap(protocol,map);

    char* token = get(map,"token");
    char* path = get(map,"path");
    char* fileName = get(map,"fileName");
    char* sha1 = get(map,"sha1");
    char* md5 = get(map,"md5");
    int fileSize = atoi(get(map,"fileSize"));

    //根据token 获取用户id
    int userId = 0;
    userId = getUserId(token);
    if(userId == -1){
        return 0;
    }
    /*获得这个用户的当前路径下是否有这个文件信息*/
    
    sql_connect_t sqlConnect;
    memset(&sqlConnect,0,sizeof(sql_connect_t));
    getConnect(&sqlConnect,pool);

    // 拼接查询sql
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
    
    
    if(rowNum > 0){// 文件或文件夹存在这个用户路径下
        file_msg_t file;
        memset(&file,0,sizeof(file_msg_t));

        MYSQL_ROW row = mysql_fetch_row(result);
        file.fileType = atoi(row[4]);
        memcpy(file.md5,row[6],strlen(row[6]));
        memcpy(file.sha1,row[7],strlen(row[7]));
        file.uploadStatus = atoi(row[8]);

        if(file.fileType == 0){
            protocolRes.status = 11;// 文件夹
        }else if(file.uploadStatus == 0 && strcmp(file.md5,md5) == 0 && strcmp(file.sha1,sha1) == 0){
            // 断点续传
            protocolRes.status = 20;// 这个用户
        }else {
            protocolRes.status = 12;// 文件名重复
        }
    }else{// 不存在这个用户下，去服务器上重新找找
        mysql_free_result(result);

        bzero(sqlBuf,sizeof(sqlBuf));
        sprintf(sqlBuf,
                "select * from file where "
                "file_hash_md5 = '%s' and file_hash_sha1 = '%s'",
                md5,sha1);
        mysql_query(&sqlConnect.connect,sqlBuf);
        result = mysql_store_result(&sqlConnect.connect);
        rowNum = mysql_num_rows(result);

        if(rowNum <= 0){
            // 新传
            protocolRes.status = 0;
        }else{
            file_msg_t file;
            memset(&file,0,sizeof(file));
            MYSQL_ROW row = mysql_fetch_row(result);
            file.uploadStatus = atoi(row[8]);
            if(file.uploadStatus == 1){
                // 秒传
                protocolRes.status = 30;
            }else{
                // 续传
                protocolRes.status = 21;// TODO:先帮助别人续传
            }
        }
    }
    mysql_free_result(result);
    /* 回复给客户端当前的状态 */
    // 先发长度，再发内容
    int sendLen = sizeof(int) * 4 + protocolRes.parameter_len;
    send(netFd,&sendLen,sizeof(int),MSG_NOSIGNAL);
    send(netFd,&protocolRes,sendLen,MSG_NOSIGNAL);
    /* 处理新传，秒传，续传 */
    int currentPathId = 0;
    bzero(sqlBuf,sizeof(sqlBuf));
    sprintf(sqlBuf,
            "select * from file where delete_flag = 0 and userId = %d and path = '%s'",
            userId,path);
    mysql_query(&sqlConnect.connect,sqlBuf);
    result = mysql_store_result(&sqlConnect.connect);
    MYSQL_ROW row;
    if(row = mysql_fetch_row(result)){
        currentPathId = atoi(row[0]);
    }
    mysql_free_result(result);
    // 根据status进行判断
    if(protocolRes.status == 0){
        // 服务器中从来没有过这个文件
        char* basePath = get(pool->map,"base_path");
        char hashFilePathName[1024] = {0};
        sprintf(hashFilePathName,"%s%s",basePath,sha1);
        int fileFd = open(hashFilePathName,O_RDWR|O_CREAT,0600);

        char filePath[200] = {0};
        sprintf(filePath,"%s%s",path,fileName);

        bzero(sqlBuf,sizeof(sqlBuf));
        sprintf(sqlBuf,
                "insert into file"
                "(user_id,path,father_path_id,file_type,file_name,file_hash_md5,file_hash_sha1,upload_status)"
                "values ( %d,  '%s', %d , 1, '%s', '%s', '%s', 0)"
                ,userId,filePath,currentPathId, fileName, md5, sha1);
        mysql_query(&sqlConnect.connect,sqlBuf);
        result = mysql_store_result(&sqlConnect.connect);
        mysql_free_result(result);

        // 调用函数接收上传的文件
        uploadFile(netFd,fileFd);
        // 上传完成后再修改上传完成标记位
        bzero(sqlBuf,sizeof(sqlBuf));
        sprintf(sqlBuf,
                "update file set upload_status = 1"
                "where path = '%s' and user_id = %d and delete_flag = 0"
                ,filePath,userId);
        mysql_query(&sqlConnect.connect, sqlBuf);
        result = mysql_store_result(&sqlConnect.connect);
        mysql_free_result(result);

        close(fileFd);
    }else if(protocolRes.status == 20){// 当前用户下的断点续传
        char* basePath = get(pool->map,"base_path");
        char hashFilePathName[1024] = {0};
        sprintf(hashFilePathName,"%s%s",basePath,sha1);
        int fileFd = open(hashFilePathName,O_RDWR);
        struct stat statFile;
        fstat(fileFd,&statFile);

        // 将文件大小转化为字符串发送给客户端
        char currentServerFileSize[20] = {0};
        sprintf(currentServerFileSize,"%ld",statFile.st_size);
        close(fileFd);
        // 回复文件大小给客户端
        memset(&protocolRes,0,sizeof(protocolRes));
        protocolRes.status = 0;
        protocolRes.parameter_flag = 1;
        protocolRes.parameter_num =1;
        //协议内容内部以键值对存储
        stringCat(protocolRes.buf,"currentServerFileSize",currentServerFileSize);

        // 将文件大小
        protocol.parameter_len = strlen(protocolRes.buf);
        int sendLen = sizeof(int)*4 + protocolRes.parameter_len;
        send(netFd,&sendLen,sizeof(int),MSG_NOSIGNAL);
        send(netFd,&protocolRes,sendLen,MSG_NOSIGNAL);
        // 开始进行断点续传
        FILE* file = fopen(hashFilePathName,"a+");
        uploadFileBreakPoint(netFd,file);
        fclose(file);

        // 再修改为上传完成
        char filePath[200] = {0};
        sprintf(filePath,"%s%s",path,fileName);
        
        bzero(sqlBuf,sizeof(sqlBuf));
        sprintf(sqlBuf,
                "update file set upload_status = 1"
                "where path = '%s' and user_id = %d and delete_flag = 0"
                ,filePath,userId);
        mysql_query(&sqlConnect.connect, sqlBuf);
        result = mysql_store_result(&sqlConnect.connect);
        mysql_free_result(result);
    }else if(protocolRes.status == 21){
        // 非当前用户下的断点续传
        char* basePath = get(pool->map,"base_path");
        char hashFilePathName[1024] = {0};
        sprintf(hashFilePathName,"%s%s",basePath,sha1);
        int fileFd = open(hashFilePathName,O_RDWR);
        struct stat statFile;
        fstat(fileFd,&statFile);

        // 将文件大小转化为字符串发送给客户端
        char currentServerFileSize[20] = {0};
        sprintf(currentServerFileSize,"%ld",statFile.st_size);
        close(fileFd);
        // 回复文件大小给客户端
        memset(&protocolRes,0,sizeof(protocolRes));
        protocolRes.status = 0;//TODO:为什么状态还是0
        protocolRes.parameter_flag = 1;
        protocolRes.parameter_num =1;
        //协议内容内部以键值对存储
        stringCat(protocolRes.buf,"currentServerFileSize",currentServerFileSize);

        // 将文件大小
        protocol.parameter_len = strlen(protocolRes.buf);
        int sendLen = sizeof(int)*4 + protocolRes.parameter_len;
        send(netFd,&sendLen,sizeof(int),MSG_NOSIGNAL);
        send(netFd,&protocolRes,sendLen,MSG_NOSIGNAL);
        // 开始进行断点续传
        FILE* file = fopen(hashFilePathName,"a+");
        uploadFileBreakPoint(netFd,file);
        fclose(file);

        // 再修改为上传完成
        bzero(sqlBuf,sizeof(sqlBuf));
        sprintf(sqlBuf,
                "update file set upload_status = 1"
                "where file_hash_md5 = '%s' and file_hash_sha1 = '%s' and upload_status = 0"
                ,md5,sha1);
        mysql_query(&sqlConnect.connect, sqlBuf);
        result = mysql_store_result(&sqlConnect.connect);
        mysql_free_result(result);

        char filePath[200] = {0};
        sprintf(filePath,"%s%s",path,fileName);
        bzero(sqlBuf,sizeof(sqlBuf));
        sprintf(sqlBuf,
                "insert into file"
                "(user_id,path,father_path_id,file_type,file_name,file_hash_md5,file_hash_sha1,upload_status)"
                "values ( %d,  '%s', %d , 1, '%s', '%s', '%s', 1)"
                ,userId,filePath,currentPathId,fileName,md5,sha1);
        mysql_query(&sqlConnect.connect,sqlBuf);
        result = mysql_store_result(&sqlConnect.connect);
        mysql_free_result(result);

    }else if(protocolRes.status == 30){
        // 秒传逻辑，直接向数据库添加数据
        char filePath[200] = {0};
        sprintf(filePath,"%s%s",path,fileName);
        bzero(sqlBuf,sizeof(sqlBuf));
        sprintf(sqlBuf,
                "insert into file"
                "(user_id,path,father_path_id,file_type,file_name,file_hash_md5,file_hash_sha1,upload_status)"
                "values ( %d,  '%s', %d , 1, '%s', '%s', '%s', 1)"
                ,userId,filePath,currentPathId,fileName,md5,sha1);
        mysql_query(&sqlConnect.connect,sqlBuf);
        result = mysql_store_result(&sqlConnect.connect);
        mysql_free_result(result);
    }
    /* 使用完成之后将数据库连接归还 */
    setConnect(&sqlConnect,pool);

    return 0;
}
