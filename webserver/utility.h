/*
*Author		Zou Xiao hang
*Email		1210603696@qq.com
*File Name	utility.h
*Date 		2013/10/05
*/
#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <strings.h>
#include <string>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <utility>
#include <fstream>
#include <sstream>
#include <map>
#include <iostream>
#include <string.h>
#include <pthread.h>
#include <netinet/tcp.h>
#include <time.h>
#include <sys/stat.h>

using namespace std;

extern string tyhp_docroot;
#define TYHP_DOCROOT	1
extern string tyhp_domain;
#define TYHP_DOMAIN 	2

/***********************************  项目实用工具函数  *******************************************/
/*
 *函数作用：得到系统时间
 *函数参数：无
 *函数返回值: 系统时间 例如：Fri, 22 May 2009 06:07:21 GMT
 */
string tyhp_time_gmt();
/*
 *函数作用：根据http请求包中的url和配置文件中的docroot配置选项构造真正的url
 *函数参数：url
 *函数返回值: 真正的url(绝对路径)
 */
string tyhp_make_real_url(const string& url);
/*
 *函数作用：测试文件是否存在
 *函数参数：path为绝对路径+文件名
 *函数返回值: -1表示文件不存在，其他值表示文件存在
 */
inline int tyhp_is_file_existed(const char *path)
{
	int ret = open(path, O_RDONLY | O_EXCL);
	close(ret);
	return ret;
}
/*
 *函数作用：获得文件长度
 *函数参数：path为绝对路径+文件名
 *函数返回值: 文件长度
 */
int tyhp_get_file_length(const char *path);
 /*
 *函数作用：获得文件最后修改时间
 *函数参数：path为绝对路径+文件名
 *函数返回值: 文件最后修改时间
 */
 string tyhp_get_file_modified_time(const char *path);
 /*
 *函数作用：初始化全局变量tyhp_config_keyword_map，必须在使用tyhp_config_keyword_map前调用此函数
 *函数参数：无
 *函数返回值: 无
 */
 void tyhp_init_config_keyword_map();
/*
 *函数作用：解析配置文件
 *函数参数：path为绝对路径+文件名
 *函数返回值: -1表示解析失败，0代表解析成功
 */
 int tyhp_parse_config(const char *path);
 /*
 *函数作用：设置文件描述符为非阻塞模式
 *函数参数：要设置的描述符
 *函数返回值: 无
 */
 void tyhp_set_nonblocking(int fd);
 /*
 *函数作用：设置套接字SO_REUSEADDR选项
 *函数参数：要设置的套接字
 *函数返回值: 无
 */
 void tyhp_set_reuse_addr(int sockfd);
 /*
 *函数作用：开启套接字TCP_NODELAY选项，关闭nagle算法
 *函数参数：要设置的套接字
 *函数返回值: 无
 */
 void tyhp_set_off_tcp_nagle(int sockfd);
 /*
 *函数作用：关闭套接字TCP_NODELAY选项，开启nagle算法
 *函数参数：要设置的套接字
 *函数返回值: 无
 */
 void tyhp_set_on_tcp_nagle(int sockfd);
/*
 *函数作用：开启套接字TCP_CORK选项
 *函数参数：要设置的套接字
 *函数返回值: 无
 */
 void tyhp_set_on_tcp_cork(int sockfd);
 /*
 *函数作用：关闭套接字TCP_CORK选项
 *函数参数：要设置的套接字
 *函数返回值: 无
 */
 void tyhp_set_off_tcp_cork(int sockfd);
/*
 *函数作用：设置套接字SO_RCVTIMEO选项，接收超时
 *函数参数：sockfd要设置的套接字, sec秒, usec毫秒
 *函数返回值: 无
 */
 void tyhp_set_recv_timeo(int sockfd, int sec, int usec);
/*
 *函数作用：设置套接字SO_SNDTIMEO选项，发送超时
 *函数参数：sockfd要设置的套接字, sec秒, usec毫秒
 *函数返回值: 无
 */
 void tyhp_set_snd_timeo(int sockfd, int sec, int usec);
/******************************************************************************************/

/***********************************  系统函数的包裹函数  *******************************************/
int tyhp_socket(int domain, int type, int protocol);
void tyhp_listen(int sockfd, int backlog);
void tyhp_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int tyhp_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
struct servent* tyhp_getservbyname(const char *name, const char *proto);
int tyhp_epoll_create(int size);
void tyhp_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int tyhp_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

void *tyhp_calloc(size_t nmemb, size_t size);
void *tyhp_malloc(size_t size);
void tyhp_free(void *ptr);
/******************************************************************************************/
#endif