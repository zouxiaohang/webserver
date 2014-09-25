/*
*Author		Zou Xiao hang
*Email		1210603696@qq.com
*File Name	tinyhttp.h
*Date 		2013/10/05
*/
#ifndef _TINY_HTTP_H_
#define	_TINY_HTTP_H_

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
#include <sys/sendfile.h>

#include "utility.h"
#include "parse.h"

using namespace std;

typedef struct _epollfd_connfd
{
	int epollfd;
	int connfd;
}_epollfd_connfd;

/***********************************  常数定义  *******************************************/
#define MAX_EVENTS	1024	//epoll最大监听事件数
#define MAX_BACKLOG	100		//监听队列最大数
/*****************************************************************************************/

/***********************************  保存配置文件相关值  *********************************/
string tyhp_domain("");
string tyhp_docroot(""); 
/*****************************************************************************************/

/***********************************  MIME定义  *******************************************/
typedef struct mime_node
{
	const char *type;
	const char *value;
}mime_node;

mime_node tyhp_mime[] = 
{
	{".html", "text/html"},
	{".xml", "text/xml"},
	{".xhtml", "application/xhtml+xml"},
	{".txt", "text/plain"},
	{".rtf", "application/rtf"},
	{".pdf", "application/pdf"},
	{".word", "application/msword"},
	{".png", "image/png"},
	{".gif", "image/gif"},
	{".jpg", "image/jpeg"},
	{".jpeg", "image/jpeg"},
	{".au", "audio/basic"},
	{".mpeg", "video/mpeg"},
	{".mpg", "video/mpeg"},
	{".avi", "video/x-msvideo"},
	{".gz", "application/x-gzip"},
	{".tar", "application/x-tar"},
	{NULL ,NULL}
};
/*
 *函数作用：将MIME的type转换为相应的value
 *函数参数：type
 *函数返回值: NULL表示type在MIME中不存在，否则返回相应value的指针
 */
inline const char* tyhp_mime_type2value(const char *type)
{
	for(int i = 0; tyhp_mime[i].type != NULL; ++i)
	{
		if(strcmp(type, tyhp_mime[i].type) == 0)
			return tyhp_mime[i].value;
	}
	return NULL;
}
/*****************************************************************************************/

/***********************************  HTTP状态码  *******************************************/
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

char tyhp_ok[] = 			"OK";
char tyhp_badrequest[] = 	"Bad Request";
char tyhp_forbidden[] =		"Forbidden";
char tyhp_notfound[] = 		"Not Found";
char tyhp_noimplemented[] = 	"No Implemented";

/*
 *函数作用：通过HTTP状态码返回友好语句
 *函数参数：HTTP状态码
 *函数返回值: 相应的语句
 */
char *tyhp_get_state_by_codes(int http_codes);
/******************************************************************************************/

/***********************************  HTTP响应首部  *******************************************/
#define TYHP_ACCEPTRANGE_HEAD			"Accpet-Range"
#define	TYHP_AGE_HEAD 					"Age"
#define	TYHP_ALLOW_HEAD					"Allow"
#define	TYHP_CONTENTBASE_HEAD			"Content-Base"
#define	TYHP_CONTENTLENGTH_HEAD			"Content-Length"
#define	TYHP_CONTENTLOCATION_HEAD		"Content-Location"
#define	TYHP_CONTENTRANGE_HEAD			"Content-Range"
#define	TYHP_CONTENTTYPE_HEAD			"Content-Type"
#define	TYHP_DATE_HEAD					"Date"
#define	TYHP_EXPIRES_HEAD				"Expires"
#define	TYHP_LAST_MODIFIED_HEAD			"Last-Modified"
#define	TYHP_LOCATION_HEAD 				"Location"
#define	TYHP_PUBLIC_HEAD				"Public"
#define TYHP_RANGE_HEAD 				"Range"
#define	TYHP_SERVER_HEAD				"Server"
/******************************************************************************************/
/***********************************  HTTP请求首部  *******************************************/
//#define TYHP
/******************************************************************************************/

#endif