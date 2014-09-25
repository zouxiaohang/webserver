/*
*Author		Zou Xiao hang
*Email		1210603696@qq.com
*File Name	tinyhttp.cpp
*Date 		2013/10/05
*/
#include "tinyhttp.h"	

#define ONEKILO		1024
#define ONEMEGA		1024*ONEKILO
#define ONEGIGA		1024*ONEMEGA

/***********************************  线程相关  *******************************************/
/*
 *函数作用：处理客户端链接的线程例程
 *函数参数：param为客户conn_sock
 *函数返回值: 无
 */
void* tyhp_thread_func(void *param);


//记录当前处理线程的数量
int32_t tyhp_thread_num = 0;
pthread_mutex_t tyhp_thread_num_mutex = PTHREAD_MUTEX_INITIALIZER;
/*
 *函数作用：tyhp_thread_num原子加1
 *函数参数：无
 *函数返回值: 无
 */
void tyhp_thread_num_add1();
/*
 *函数作用：tyhp_thread_num原子减1
 *函数参数：无
 *函数返回值: 无
 */
void tyhp_thread_num_minus1();
/*
 *函数作用：tyhp_thread_num原子读
 *函数参数：无
 *函数返回值: tyhp_thread_num当前值
 */
 int32_t tyhp_thread_num_read();
/*****************************************************************************************/

/********************************  tyhp_http_header_t处理函数  *********************************/
 /*
 *函数作用：根据解析下来的tyhp_http_header_t来处理客户的请求
 *函数参数：  phttphdr指向要处理的tyhp_http_header_t
 			out保存了处理的结果，即http响应包
 *函数返回值: HTTP状态码
 */
int tyhp_do_http_header(tyhp_http_header_t *phttphdr, string& out);
/*
 *函数作用：通过HTTP状态码返回友好语句
 *函数参数：HTTP状态码
 *函数返回值: 相应的语句
 */
char *tyhp_get_state_by_codes(int http_codes);
/*****************************************************************************************/

/***********************************  web服务器程序入口函数  ***************************************/
int main(int argc, char const *argv[])
{
	int 				listen_fd;
	int 				conn_sock; 
	int 				nfds;
	int 				epollfd;
	uint16_t 			listen_port;
	struct servent 		*pservent;
	struct epoll_event 	ev;
	struct epoll_event	events[MAX_EVENTS];
	struct sockaddr_in 	server_addr;
	struct sockaddr_in	client_addr;
	socklen_t 			addrlen;
	pthread_attr_t		pthread_attr_detach;
	_epollfd_connfd 	epollfd_connfd;
	pthread_t 			tid;

	if(argc != 2)
	{
		printf("Usage: %s <config_path>\n", argv[0]);
		exit(-1);
	}
	//判断配置文件是否存在
	if(-1 == tyhp_is_file_existed(argv[1]))
	{
		perror("tyhp_is_file_existed");
		exit(-1);
	}
	//调用tyhp_parse_config解析配置文件
	if(-1 == tyhp_parse_config(argv[1]))
	{
		printf("tyhp_parse_config error\n");
		exit(-1);
	}

	//创建监听套接字
	listen_fd = tyhp_socket(AF_INET, SOCK_STREAM, 0);
	//设置监听套接字为非阻塞模式
	tyhp_set_nonblocking(listen_fd);
	//对监听套接字设置SO_REUSEADDR选项
	tyhp_set_reuse_addr(listen_fd);
	//通过服务名和协议名获得相应的知名端口，其实可以直接设置端口为80，我们这样做是为了可扩展性
	pservent = tyhp_getservbyname("http", "tcp");
	//pservent->s_port已经是网络字节序了
	listen_port = pservent->s_port;
	
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = (listen_port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//将服务器sockaddr_in与监听套接字绑定
	tyhp_bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	//开始监听
	tyhp_listen(listen_fd, MAX_BACKLOG);
	
	//创建epoll文件描述符
	epollfd = tyhp_epoll_create(MAX_EVENTS);
	
	ev.events = EPOLLIN;//可读事件
	ev.data.fd = listen_fd;
	//将监听事件加入epoll中
	tyhp_epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_fd, &ev);

	//设置线程属性为detach
	pthread_attr_init(&pthread_attr_detach);
	pthread_attr_setdetachstate(&pthread_attr_detach, PTHREAD_CREATE_DETACHED);
		
	for(;;)
	{
		//无限等待直到有描述符就绪
		nfds = tyhp_epoll_wait(epollfd, events, MAX_EVENTS, -1);
		//若tyhp_epoll_wait被中断则重新调用该函数
		if(nfds == -1 && errno == EINTR)
			continue;
			
		for(int n = 0; n != nfds; ++n)
		{
			//处理监听套接字触发的事件
			if(events[n].data.fd == listen_fd)
			{
				conn_sock = tyhp_accept(listen_fd, (struct sockaddr*)&client_addr, &addrlen);
				//设置新链接上的套接字为非阻塞模式
				tyhp_set_nonblocking(conn_sock);
				//设置读事件和ET模式
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = conn_sock;
				//将监听事件加入epoll中
				tyhp_epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev);
			}
			else
			{
				epollfd_connfd.epollfd = epollfd;
				epollfd_connfd.connfd = events[n].data.fd;
				ev.data.fd = conn_sock;
				//epoll不再监听这个客户端套接字
				tyhp_epoll_ctl(epollfd, EPOLL_CTL_DEL, conn_sock, &ev);
				//处理链接
				pthread_create(&tid, &pthread_attr_detach, &tyhp_thread_func, (void*)&epollfd_connfd);
				//tyhp_thread_func((void*)&epollfd_connfd);
				//close(conn_sock);
			}
		}
	}
	//清理工作
	pthread_attr_destroy(&pthread_attr_detach);

	//关闭监听套接字
	close(listen_fd);
	return 0;
}
/*****************************************************************************************/

/***********************************  线程相关  *******************************************/
/*
 *函数作用：处理客户端链接的线程例程
 *函数参数：
 *函数返回值: NULL
 */
 #define TIMEOUT	1000*60*4 //设置超时 milliseconds
void* tyhp_thread_func(void *param)
{
	tyhp_thread_num_add1();

	tyhp_http_header_t *phttphdr = tyhp_alloc_http_header();

	_epollfd_connfd *ptr_epollfd_connfd = (_epollfd_connfd*)param;
	//int epollfd = ptr_epollfd_connfd->epollfd;
	//获取客户连接socket
	int conn_sock = ptr_epollfd_connfd->connfd;

	struct epoll_event ev, events[2];
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = conn_sock;
	//针对客户链接的新epollfd
	int epollfd = tyhp_epoll_create(2);
	tyhp_epoll_ctl(epollfd, EPOLL_CTL_ADD, ev.data.fd, &ev);
	int nfds = 0;

	pthread_t tid = pthread_self();
	printf("NO.%u thread runs now !!!\n", (unsigned int)tid);
	
	//Nginx默认http请求包大小为1M，所以我也分配1M缓存来存http请求包
	char *buff = (char*)tyhp_malloc(ONEMEGA);
	bzero(buff, ONEMEGA);

	//关闭connfd的Nagle算法
	tyhp_set_off_tcp_nagle(conn_sock);
	//设置接收超时时间为60秒
	tyhp_set_recv_timeo(conn_sock, 60, 0);
	//设置发送超时时间为120秒
	//tyhp_set_snd_timeo(connfd, 120, 0);
begin:
	int32_t nread = 0, n = 0;
	for(;;)
	{	
		if((n = read(conn_sock, buff+nread, ONEMEGA-1)) > 0)
			nread += n;
		else if(0 == n)
			break;
		else if(-1 == n && errno == EINTR)
			continue;
		else if(-1 == n && errno == EAGAIN)
			break;
		else if(-1 == n && errno == EWOULDBLOCK)
		{
			perror("socket read timeout");
			goto out;
		}
		else
		{
			perror("read http request error");
			tyhp_free(buff);
			break;
		}
		
	}

	if(0 != nread)
	{
		string str_http_request(buff, buff + nread);

		//do_something(str_http_request);
		if(!tyhp_parse_http_request(str_http_request, phttphdr))
		{
			perror("tyhp_parse_http_request: parse str_http_request failed");
			goto out;
		}
		cout<<"解析出来的http请求包:"<<endl;
		tyhp_print_http_header(phttphdr);

		string out;
		int http_codes = tyhp_do_http_header(phttphdr, out);

		/****** debug *****/
		cout<<"http响应包:"<<endl<<out<<endl;

		char *out_buf = (char *)tyhp_malloc(out.size());
		if(out_buf == NULL)
			goto out;
		int i;
		for(i = 0; i != out.size(); ++i)
			out_buf[i] = out[i];
		out_buf[i] = '\0';
		int nwrite = 0; n = 0;
		if( http_codes == TYHP_BADREQUEST 		|| 
			http_codes == TYHP_NOIMPLEMENTED 	||
			http_codes == TYHP_NOTFOUND 		||
			(http_codes == TYHP_OK && phttphdr->method == "HEAD"))
		{
			while((n = write(conn_sock, out_buf + nwrite, i)) != 0)
			{
				if(n == -1 && errno == EINTR)
					continue;
				else
					goto out;
				nwrite += n;
			}
		}
		if(http_codes == TYHP_OK)
		{
			if(phttphdr->method == "GET")
			{
				while((n = write(conn_sock, out_buf + nwrite, i)) != 0)
				{
					cout<<n<<endl;
					if(n == -1 && errno == EINTR)
						continue;
					else
						break;
					nwrite += n;
				}
				string real_url = tyhp_make_real_url(phttphdr->url);
				int fd = open(real_url.c_str(), O_RDONLY);
				int file_size = tyhp_get_file_length(real_url.c_str());
				cout<<"file size "<<file_size<<endl;
				int nwrite = 0;
				cout<<"sendfile : "<<real_url.c_str()<<endl;
			again:
				if((sendfile(conn_sock, fd, (off_t*)&nwrite, file_size)) < 0)
					perror("sendfile");
				if(nwrite < file_size)
					goto again;
				cout<<"sendfile ok:"<<nwrite<<endl;
			}
		}
		free(out_buf);
		//超时4分钟
		nfds = tyhp_epoll_wait(epollfd, events, 2, TIMEOUT);
		if(0 == nfds)//timeout
			goto out;
		for(int i = 0; i < nfds; ++i)
		{
			if(events[i].data.fd == conn_sock)
				goto begin;
			else
				goto out;
		}
	}

out:
	tyhp_free_http_header(phttphdr);
	close(conn_sock);
	tyhp_thread_num_minus1();
	printf("NO.%u thread ends now ~~~\n", (unsigned int)tid);
}

/*
 *函数作用：tyhp_thread_num原子加1
 *函数参数：无
 *函数返回值: 无
 */
void tyhp_thread_num_add1()
{
	pthread_mutex_lock(&tyhp_thread_num_mutex);
	++tyhp_thread_num;
	pthread_mutex_unlock(&tyhp_thread_num_mutex);
}
/*
 *函数作用：tyhp_thread_num原子减1
 *函数参数：无
 *函数返回值: 无
 */
void tyhp_thread_num_minus1()
{
	pthread_mutex_lock(&tyhp_thread_num_mutex);
	--tyhp_thread_num;
	pthread_mutex_unlock(&tyhp_thread_num_mutex);
}
/*
 *函数作用：tyhp_thread_num原子读
 *函数参数：无
 *函数返回值: tyhp_thread_num当前值
 */
 int32_t tyhp_thread_num_read();
 /*****************************************************************************************/

 /********************************  tyhp_http_header_t处理函数  *********************************/
 /*
 *函数作用：根据解析下来的tyhp_http_header_t来处理客户的请求
 *函数参数：  phttphdr指向要处理的tyhp_http_header_t
 			out保存了处理的结果，即http响应包
 *函数返回值: HTTP状态码

 *目前支持的请求首部：
 *目前支持的响应首部：Date，Content-Base，Content-Length，Content-Location
 						Last-Modified，Public，Server
 */
int tyhp_do_http_header(tyhp_http_header_t *phttphdr, string& out)
{
	char status_line[256] = {0};
	string crlf("\r\n");
	string server("Server: tinyhttp\r\n");
	string Public("Public: GET, HEAD\r\n");
	string content_base = "Content-Base: " + tyhp_domain + crlf;
	string date = "Date:" + tyhp_time_gmt() + crlf;

	string content_length("Content-Length: ");
	string content_location("Content-Location: ");
	string last_modified("Last-Modified: ");
	//string body("");

	if(phttphdr == NULL)
	{
		snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
			TYHP_BADREQUEST, tyhp_get_state_by_codes(TYHP_BADREQUEST));
		out = status_line + crlf;
		return TYHP_BADREQUEST;
	}

	string method = phttphdr->method;
	string real_url = tyhp_make_real_url(phttphdr->url);
	string version = phttphdr->version;
	if(method == "GET" || method == "HEAD")
	{
		if(tyhp_is_file_existed(real_url.c_str()) == -1)
		{
			snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
				TYHP_NOTFOUND, tyhp_get_state_by_codes(TYHP_NOTFOUND));
			out += (status_line + server + date + crlf); 
			return TYHP_NOTFOUND;
		}
		else
		{
			int len = tyhp_get_file_length(real_url.c_str());
			snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
				TYHP_OK, tyhp_get_state_by_codes(TYHP_OK));
			out += status_line;
			snprintf(status_line, sizeof(status_line), "%d\r\n", len);
			out += content_length + status_line;
			out += server + content_base + date;
			out += last_modified + tyhp_get_file_modified_time(real_url.c_str()) + crlf + crlf;
		}
	}
	else if(method == "PUT")
	{
		snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
				TYHP_NOIMPLEMENTED, tyhp_get_state_by_codes(TYHP_NOIMPLEMENTED));
		out += status_line + server + Public + date + crlf;
		return TYHP_NOIMPLEMENTED;
	}
	else if(method == "DELETE")
	{
		snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
				TYHP_NOIMPLEMENTED, tyhp_get_state_by_codes(TYHP_NOIMPLEMENTED));
		out += status_line + server + Public + date + crlf;
		return TYHP_NOIMPLEMENTED;
	}
	else if(method == "POST")
	{
		snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
				TYHP_NOIMPLEMENTED, tyhp_get_state_by_codes(TYHP_NOIMPLEMENTED));
		out += status_line + server + Public + date + crlf;
		return TYHP_NOIMPLEMENTED;
	}
	else
	{
		snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
			TYHP_BADREQUEST, tyhp_get_state_by_codes(TYHP_BADREQUEST));
		out = status_line + crlf;
		return TYHP_BADREQUEST;
	}

	return TYHP_OK;
}
/*
#define TYHP_CONTINUE 		100	//收到了请求的起始部分，客户端应该继续请求

#define TYHP_OK				200	//服务器已经成功处理请求
#define TYHP_ACCEPTED		202	//请求已接受，服务器尚未处理

#define TYHP_MOVED			301	//请求的URL已移走，响应应该包含Location URL
#define	TYHP_FOUND			302	//请求的URL临时移走，响应应该包含Location URL
#define TYHP_SEEOTHER		303	//告诉客户端应该用另一个URL获取资源，响应应该包含Location URL
#define TYHP_NOTMODIFIED	304	//资源未发生变化

#define TYHP_BADREQUEST		400	//客户端发送了一条异常请求
#define TYHP_FORBIDDEN		403	//服务器拒绝请求
#define TYHP_NOTFOUND		404	//URL未找到

#define TYHP_ERROR			500	//服务器出错
#define TYHP_NOIMPLEMENTED	501 //服务器不支持当前请求所需要的某个功能
#define TYHP_BADGATEWAY		502	//作为代理或网关使用的服务器遇到了来自响应链中上游的无效响应
#define TYHP_SRVUNAVAILABLE	503 //服务器目前无法提供请求服务，过一段时间后可以恢复

/*
 *函数作用：通过HTTP状态码返回友好语句
 *函数参数：HTTP状态码
 *函数返回值: 相应的语句
 */
char *tyhp_get_state_by_codes(int http_codes)
{
	switch (http_codes)
	{
		case TYHP_OK:
			return tyhp_ok;
		case TYHP_BADREQUEST:
			return tyhp_badrequest;
		case TYHP_FORBIDDEN:
			return tyhp_forbidden;
		case TYHP_NOTFOUND:
			return tyhp_notfound;
		case TYHP_NOIMPLEMENTED:
			return tyhp_noimplemented;
		default:
			break;
	}

	return NULL;
}
/*****************************************************************************************/