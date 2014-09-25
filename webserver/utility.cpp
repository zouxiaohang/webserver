/*
*Author		Zou Xiao hang
*Email		1210603696@qq.com
*File Name	utility.cpp
*Date 		2013/10/05
*/
#include "utility.h"

//存储解析文件的键值对，例如key=docroot value=/home/zxh/desktop/code/webserver
map<string, int> tyhp_config_keyword_map;

/***********************************  项目实用工具函数  *******************************************/
/*
 *函数作用：得到系统时间
 *函数参数：无
 *函数返回值: 系统时间 例如：Fri, 22 May 2009 06:07:21 GMT
 */
string tyhp_time_gmt()
{
	time_t now;
	struct tm *time_now;
	string str_time;

	time(&now);
	time_now = localtime(&now);

	switch(time_now->tm_wday)
	{
		case 0:
			str_time += "Sun, ";
			break;
		case 1:
			str_time += "Mon, ";
			break;
		case 2:
			str_time += "Tue, ";
			break;
		case 3:
			str_time += "Wed, ";
			break;
		case 4:
			str_time += "Thu, ";
			break;
		case 5:
			str_time += "Fri, ";
			break;
		case 6:
			str_time += "Sat, ";
			break;
	}
	char buf[16];
	snprintf(buf, sizeof(buf), "%d ", time_now->tm_mday);
	str_time += string(buf);
	switch(time_now->tm_mon)
	{
		case 0:
			str_time += "Jan ";
			break;
		case 1:
			str_time += "Feb ";
			break;
		case 2:
			str_time += "Mar ";
			break;
		case 3:
			str_time += "Apr ";
			break;
		case 4:
			str_time += "May ";
			break;
		case 5:
			str_time += "Jun ";
			break;
		case 6:
			str_time += "Jul ";
			break;
		case 7:
			str_time += "Aug ";
			break;
		case 8:
			str_time += "Sep ";
			break;
		case 9:
			str_time += "Oct ";
			break;
		case 10:
			str_time += "Nov ";
			break;
		case 11:
			str_time += "Dec ";
			break;
	}
	snprintf(buf, sizeof(buf), "%d", time_now->tm_year + 1900);
	str_time += string(buf);
	snprintf(buf, sizeof(buf), " %d:%d:%d ", time_now->tm_hour, time_now->tm_min, time_now->tm_sec);
	str_time += string(buf);

	str_time += "GMT";

	return str_time;
}
/*
 *函数作用：根据http请求包中的url和配置文件中的docroot配置选项构造真正的url
 *函数参数：url
 *函数返回值: 真正的url(绝对路径)
 */
string tyhp_make_real_url(const string& url)
{
	string real_url, url2;

	int n = 0;
	
	if((n = url.find(tyhp_domain, 0)) != string::npos)//url中包含域名，要将其删去
		url2 = url.substr(tyhp_domain.size(), url.size() - tyhp_domain.size());
	else
		url2 = url;

	if(tyhp_docroot[tyhp_docroot.size() - 1] == '/')//配置项docroot末尾有'/'
	{
		if(url2[0] == '/')
			real_url = tyhp_docroot + url2.erase(0, 1);
		else
			real_url = tyhp_docroot + url2;
	}
	else//配置项docroot末尾没有'\'
	{
		if(url2[0] == '/')
			real_url = tyhp_docroot + url2;
		else
			real_url = tyhp_docroot + '/' + url2;
	}

	return real_url;
}
/*
 *函数作用：获得文件长度
 *函数参数：path为绝对路径+文件名
 *函数返回值: 文件长度
 */
 int tyhp_get_file_length(const char *path)
 {
 	struct stat buf;
 	int ret = stat(path, &buf);
 	if(ret == -1)
 	{
 		perror("tyhp_get_file_length");
 		exit(-1);
 	}
 	return (int)buf.st_size;
 }
  /*
 *函数作用：获得文件最后修改时间
 *函数参数：path为绝对路径+文件名
 *函数返回值: 文件最后修改时间
 */
 string tyhp_get_file_modified_time(const char *path)
 {
 	struct stat buf;
 	int ret = stat(path, &buf);
 	if(ret == -1)
 	{
 		perror("tyhp_get_file_length");
 		exit(-1);
 	}
 	char array[32] = {0};
 	snprintf(array, sizeof(array), "%s", ctime(&buf.st_mtime));
 	return string(array, array + strlen(array));
 }
/*
 *函数作用：初始化全局变量tyhp_config_keyword_map，必须在使用tyhp_config_keyword_map前调用此函数
 *函数参数：无
 *函数返回值: 无
 */
 void tyhp_init_config_keyword_map()
 {
 	tyhp_config_keyword_map.insert(make_pair("docroot", TYHP_DOCROOT));
 	tyhp_config_keyword_map.insert(make_pair("domain", TYHP_DOMAIN));
 }
/*
 *函数作用：解析配置文件
 *函数参数：path为绝对路径+文件名
 *函数返回值: -1表示解析失败，0代表解析成功
 */
 int tyhp_parse_config(const char *path)
 {
 	tyhp_init_config_keyword_map();
 	int ret = 0;
 	fstream infile(path, fstream::in);
 	string line, word;
 	if(!infile)
 	{
 		printf("%s can't open\n", path);
 		infile.close();
 		return -1;
 	}
 	while(getline(infile, line))
 	{
 		stringstream stream(line);
 		stream >> word;//keyword
 		map<string, int>::const_iterator cit= tyhp_config_keyword_map.find(word);
 		if(cit == tyhp_config_keyword_map.end())
 		{
 			printf("can't find keyword\n");
 			infile.close();
 			return -1;
 		}
 		switch (cit->second)
 		{
 			case TYHP_DOCROOT:
 				stream >> tyhp_docroot;
 				break;
 			case TYHP_DOMAIN:
 				stream >> tyhp_domain;
 				break;
 			default :
 				infile.close();
 				return -1;
 		}
 	}
 	infile.close();
 	return 0;
 }
  /*
 *函数作用：设置文件描述符为非阻塞模式
 *函数参数：要设置的描述符
 *函数返回值: 无
 */
 void tyhp_set_nonblocking(int fd)
 {
	int flags = fcntl(fd, F_GETFL, 0);
	if(flags < 0)
	{
		perror("fcntl: F_GETFL");
		exit(-1);
	}
	flags |= O_NONBLOCK;
	int ret = fcntl(fd, F_SETFL, flags);
	if(ret < 0)
	{
		perror("fcntl");
		exit(-1);
	}
 }
  /*
 *函数作用：设置套接字SO_REUSEADDR选项
 *函数参数：要设置的套接字
 *函数返回值: 无
 */
 void tyhp_set_reuse_addr(int sockfd)
{
	int on = 1;
	int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if(ret == -1)
	{
		perror("setsockopt: SO_REUSEADDR");
		exit(-1);
	}
}
 /*
 *函数作用：开启套接字TCP_NODELAY选项，关闭nagle算法
 *函数参数：要设置的套接字
 *函数返回值: 无
 */
 void tyhp_set_off_tcp_nagle(int sockfd)
 {
 	int on = 1;
 	int ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
 	if(ret == -1)
 	{
 		perror("setsockopt: TCP_NODELAY ON");
		exit(-1);
 	}
 }
 /*
 *函数作用：关闭套接字TCP_NODELAY选项，开启nagle算法
 *函数参数：要设置的套接字
 *函数返回值: 无
 */
 void tyhp_set_on_tcp_nagle(int sockfd)
 {
 	int off = 0;
 	int ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &off, sizeof(off));
 	if(ret == -1)
 	{
 		perror("setsockopt: TCP_NODELAY OFF");
		exit(-1);
 	}
 }
/*
 *函数作用：开启套接字TCP_CORK选项
 *函数参数：要设置的套接字
 *函数返回值: 无
 */
 void tyhp_set_on_tcp_cork(int sockfd)
 {
 	int on = 1;
 	int ret = setsockopt(sockfd, SOL_TCP, TCP_CORK, &on, sizeof(on));
 	if(ret == -1)
 	{
 		perror("setsockopt: TCP_CORK ON");
		exit(-1);
 	}
 }
 /*
 *函数作用：关闭套接字TCP_CORK选项
 *函数参数：要设置的套接字
 *函数返回值: 无
 */
 void tyhp_set_off_tcp_cork(int sockfd)
 {
 	int off = 0;
 	int ret = setsockopt(sockfd, SOL_TCP, TCP_CORK, &off, sizeof(off));
 	if(ret == -1)
 	{
 		perror("setsockopt: TCP_CORK OFF");
		exit(-1);
 	}
 }
/*
 *函数作用：设置套接字SO_RCVTIMEO选项，接收超时
 *函数参数：sockfd要设置的套接字, sec秒, usec毫秒
 *函数返回值: 无
 */
 void tyhp_set_recv_timeo(int sockfd, int sec, int usec)
 {
 	struct timeval time= {sec, usec};
 	int ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &time, sizeof(time));
 	if(ret == -1)
 	{
 		perror("setsockopt: SO_RCVTIMEO");
		exit(-1);
 	}
 }
/*
 *函数作用：设置套接字SO_SNDTIMEO选项，发送超时
 *函数参数：sockfd要设置的套接字, sec秒, usec毫秒
 *函数返回值: 无
 */
 void tyhp_set_snd_timeo(int sockfd, int sec, int usec)
 {
 	struct timeval time= {sec, usec};
 	int ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &time, sizeof(time));
 	if(ret == -1)
 	{
 		perror("setsockopt: SO_SNDTIMEO");
		exit(-1);
 	}
 }
/******************************************************************************************/

/***********************************  系统函数的包裹函数  *******************************************/
int tyhp_socket(int domain, int type, int protocol)
{
	int listen_fd;
	if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(-1);
	}
	return listen_fd;
}
void tyhp_listen(int sockfd, int backlog)
{
	if(listen(sockfd, backlog) == -1)
	{
		perror("listen");
		exit(-1);
	}
}
void tyhp_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	if(bind(sockfd, addr, addrlen) == -1)
	{
		perror("bind");
		exit(-1);
	}
}
int tyhp_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int ret_fd = 0;
	for(;;)
	{	
		ret_fd = accept(sockfd, addr, addrlen);
		if(ret_fd > 0)
			break;
		else if(ret_fd == -1)
		{
			//由于我们把监听套接字设置为了非阻塞模式
			if(errno != EAGAIN && errno != EPROTO &&
			 		errno != EINTR && errno != ECONNABORTED)
			{	
				perror("accept");
				exit(-1);
			}
		}
		else
			continue;
	}
	return ret_fd;
}
struct servent* tyhp_getservbyname(const char *name, const char *proto)
{
	struct servent 	*pservent;
	if((pservent = getservbyname(name, proto)) == NULL)
	{
		perror("getservbyname");
		exit(-1);
	}
	return pservent;
}
int tyhp_epoll_create(int size)
{
	int epollfd;
	epollfd = epoll_create(size);
	if(-1 == epollfd)
	{
		perror("epoll_create");
		exit(-1);
	}
	return epollfd;
}
void tyhp_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
	if(epoll_ctl(epfd, op, fd, event) == -1)
	{
		perror("epoll_ctl");
		exit(-1);
	}	
}
int tyhp_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
again:
	int nfds = epoll_wait(epfd, events, maxevents, timeout);
	if(nfds == -1) //&& errno != EINTR)
	{
		if(errno != EINTR)
		{
			perror("epoll_wait");
			exit(-1);
		}
		else
			goto again;
	}
	return nfds;
}

void *tyhp_calloc(size_t nmemb, size_t size)
{
	void *ptr = calloc(nmemb, size);
	if(NULL == ptr)
	{
		perror("tyhp_calloc");
		exit(-1);
	}
	return ptr;
}
void *tyhp_malloc(size_t size)
{
	void *ptr = malloc(size);
	if(NULL == ptr)
	{
		perror("tyhp_malloc");
		exit(-1);
	}
	return ptr;
}
void tyhp_free(void *ptr)
{
	free(ptr);
}
/******************************************************************************************/