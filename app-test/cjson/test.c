/*************************************************************************
    > File Name: test.c
    > Author: baiy
    > Mail: baiyang0223@163.com 
    > Created Time: 2020-12-04-13:11:19
    > Func: 
 ************************************************************************/

#include <stdio.h>
#include "cJSON.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>


int parse_config(const char * jsbuf)
{
    cJSON       *root = NULL;
    cJSON       *node = NULL;
    root = cJSON_Parse(jsbuf);
    if ( NULL == root ) {
        printf("%s:Parse root info json failed!", __func__);
        return -1;
    }
    node = cJSON_GetObjectItem((cJSON *)root, "flag");
    if ( (NULL != node) &&  (node->type & 3))
        printf("flag is %d", node->valueint); // 逻辑值
    node = cJSON_GetObjectItem((cJSON *)root, "ip");
    if ( NULL != node  && (node->type == cJSON_String ))
        printf("ip is %s", node->valuestring); // 字符串char*
    // 数组
    cJSON * node_arr = cJSON_GetObjectItem((cJSON *)root, "arr0");
    cJSON* loop;
    int index = 0;
    cJSON_ArrayForEach(loop, node_arr){
        printf("test_arr1[%d] is %d\n",index,loop->valueint);
        index ++;
    }
    cJSON * node_arr2 = cJSON_GetObjectItem((cJSON *)root, "arr2");
    cJSON_ArrayForEach(loop, node_arr2){
        printf("test_arr2[%d]-name is %s\n",
            index,cJSON_GetObjectItem((cJSON *)loop, "name")->valuestring);
        printf("test_arr2[%d]-age is %d\n",
            index,cJSON_GetObjectItem((cJSON *)loop, "age")->valueint);
        index ++;
    }
    
    // free一定要放到所有程序最后去释放，否则其中字符串会丢失
    cJSON_free(root);
    return 0;
}


int main(int argc, char * argv[])
{
    int fd = 0;
    if(argc < 2){
        printf("input %s filename\n", argv[0]);
        return 0;
    }
    struct stat statbuf;
    int ret = stat(argv[1], &statbuf);
    if(ret < 0 || statbuf.st_size == 0){
        perror("check file failed\n");
        return -1;
    }
    fd = open(argv[1], O_RDONLY);
    if(fd < 0){
        perror("open file failed\n");
        return -1;
    }


    char *buf = malloc(statbuf.st_size);
    read(fd, buf,statbuf.st_size);
    parse_config(buf);

    free(buf);
    close(fd);
    return 0;
}
