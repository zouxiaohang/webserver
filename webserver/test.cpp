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
#include <arpa/inet.h>
#include <iostream>

using namespace std;

int main(int argc, char const *argv[])
{
	int sockfd, n;
	struct sockaddr_in servaddr;

	if(2 != argc)
		printf("usage: a.out <IP address>\n");

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("sock error\n");

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(80);
	if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
		printf("inet_pton error for %s\n", argv[1]);

	if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
		printf("connect error\n");
	
	printf("发送:\n");
	char buff[] = "GET www.zouxiaohang.com/index/index.html HTTP/1.1\r\nAccept-Language: zh-cn\r\n\r\n";
	write(sockfd, buff, sizeof(buff));
	printf("发送结束\n");
	printf("开始读取\n");
	char array[102400];
	int in = 0, nread = 0;
	while((in = read(sockfd, array+nread, sizeof(array))) > 0)
	{
		cout<<nread<<endl;
		nread += in;
		if(nread == 11510)
			break;
	}
	array[nread] = '\0';
	printf("tinyhttp web服务器返回的数据:\n%s\n%d\n", array, nread);
	printf("读取结束\n");
	return 0;
}