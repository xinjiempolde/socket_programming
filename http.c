#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAX_BUF_SIZE (1024*1024*1024)
/*
 * 将域名转换为IP地址（数字和点组成的字符串形式），例如：
 * www.baidu.com ==> 14.215.177.38
 */
int hostname_to_ip(char *hostname , char *ip)
{
	int sockfd;  
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_in *h;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;       // 使用TCP/IP协议簇
	hints.ai_socktype = SOCK_STREAM; // 建立TCP套接字

	if ( (rv = getaddrinfo( hostname , "http" , &hints , &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		h = (struct sockaddr_in *) p->ai_addr;
		strcpy(ip , inet_ntoa( h->sin_addr ) );
        // printf("%s\n", ip);
	}
	
	freeaddrinfo(servinfo); // all done with this structure
	return 0;
}

/**
 * 将HTTP响应消息中的头信息去除，返回数据信息的起始地址
 */
char *remove_header(char* data) {
    char *ret = NULL;
    if ((ret = strstr(data, "Connection: close")) == NULL) {
        return NULL;
    }
    ret += strlen("Connection: close") + 4;
    return ret;
}

/**
 * 将size个字节的数据写入到文件中
 */
int write_to_file(char *data, long size) {
    FILE *fp = fopen("back.png", "w+");
    if (fp == NULL) {
        fprintf(stderr, "opening file fails");
        return 1;
    }
    fwrite(data, size, 1, fp);
    return 0;
}

/**
 * 获取网页的HTTP响应信息，包括头信息和数据信息。
 *   hostname: 主机名，如www.baidu.com
 *   path: 路径，要获取页面的路径信息,如/hello/index.html
 *   buffer: 存储响应消息
 */
long get_page(char *hostname, char *path, char *buffer) {
    char ip[20];
    if (hostname_to_ip(hostname, ip) != 0) { // 不是有效的主机名
        return -1;
    }
    // 创建TCP连接套接字
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    // 设置服务器的IP地址和HTTP协议端口(80)
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(80); // 将本地字节序转换为网络字节序
	
	//向服务器发送连接请求
	connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    // printf("connection has established!\n");

    /*---------------------- HTTP 请求开始 ---------------------- */
	//发送并接收数据
    snprintf(buffer, 64, "GET %s HTTP/1.1\r\n", path);
	write(sock, buffer, strlen(buffer));

    snprintf(buffer, 64, "Host: %s\r\n", hostname);
    write(sock, buffer, strlen(buffer));

    snprintf(buffer, 64, "%s", "Connection: close\r\n");
    write(sock, buffer, strlen(buffer));

    write(sock, "\r\n", strlen("\r\n"));
    /*---------------------- HTTP 请求结束 ---------------------- */

    /* 获取HTTP响应消息 */
	memset(buffer, 0, MAX_BUF_SIZE);
    long tobytes = 0; // total bytes
    int rebytes; // read bytes
    while ((rebytes = read(sock, buffer, 1024)) > 0) { // read the remains
        buffer += rebytes;
        tobytes += rebytes;
    }
	//断开连接
	close(sock);
	return tobytes;
}


int main(int argc, char **argv){
	char* buffer = (char*)malloc(sizeof(char) * MAX_BUF_SIZE);
    long tobytes = get_page("www.baidu.com", "/img/PCtm_d9c8750bed0b3c7d089fa7d55720d6cf.png", buffer);
    // printf("%s\n", buffer);
    char *img_data = remove_header(buffer);
    write_to_file(img_data, tobytes - (img_data-buffer));
	return 0;
}

