webserver
=========
###简易的http服务器
####1.服务器的架构为epoll + 多线程 + sendfile
#####2.暂时只支持GET和HEAD方法，支持的首部不多大概七八个吧，支持伪长连接
#####3.可配置的，现阶段只支持domain和docroot配置项，domain就是你部署的网站域名，docroot想必学过网页的都知道是什么意思吧~

详见 http://www.cnblogs.com/zxh1210603696/p/3371715.html
