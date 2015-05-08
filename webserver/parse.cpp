/*
*Author		Zou Xiao hang
*Email		1210603696@qq.com
*File Name	parse.h
*Date 		2013/10/05
*/
#include "parse.h"

/*
 *函数作用：打印tyhp_http_header_t里的header
 *函数参数：tyhp_header 的const 引用
 *函数返回值: 无
 */
 void tyhp_print_http_header_header(const tyhp_header& head)
 {
 	if(!head.empty())
 	{
 		tyhp_header::const_iterator cit = head.begin();
 		while(cit != head.end())
 		{
 			cout<<cit->first<<":"<<cit->second<<endl;
 			++cit;
 		}
 	}
 }
/*
 *函数作用：打印tyhp_http_header_t
 *函数参数：tyhp_http_header_t指针
 *函数返回值: 无
 */
 void tyhp_print_http_header(tyhp_http_header_t *phttphdr)
 {
 	if(NULL == phttphdr)
 	{
 		perror("phttphdr == NULL");
 		return ;
 	}

 	cout<<phttphdr->method<<" "<<phttphdr->url<<" "<<phttphdr->version<<endl;
	tyhp_print_http_header_header(phttphdr->header);
	cout<<endl<<phttphdr->body<<endl;
 }


/*
 *函数作用：分配内存给tyhp_http_header_t
 *函数参数：无
 *函数返回值: NULL表示分配失败，其他值表示成功
 */
tyhp_http_header_t *tyhp_alloc_http_header()
{
	tyhp_http_header_t *phttphdr = (tyhp_http_header_t *)new tyhp_http_header_t;
	if(phttphdr == NULL)
	{
		perror("tyhp_alloc_http_header");
		exit(-1);
	}
	return phttphdr;
}

/*
 *函数作用：回收分配给tyhp_http_header_t的内存
 *函数参数：tyhp_http_header_t指针
 *函数返回值: 无
 */
void tyhp_free_http_header(tyhp_http_header_t *phttphdr)
{
	if(phttphdr == NULL)
		return ;
	delete phttphdr;
}

/*
 *函数作用：解析http_request
 *函数参数：http_request为待解析的值，phttphdr保存了解析下来的值
 *函数返回值: true表示解析成功，false表示解析失败
 */
bool tyhp_parse_http_request(const string& http_request, tyhp_http_header_t *phttphdr)
{
	if(http_request.empty())
	{
		perror("tyhp_parse_http_request: http_request is empty");
		return false;
	}
	if(phttphdr == NULL)
	{
		perror("tyhp_parse_http_request: phttphdr is NULL");
		return false;
	}

	string crlf("\r\n"), crlfcrlf("\r\n\r\n");
	int prev = 0, next = 0;

	//解析http请求包的起始行
	if((next = http_request.find(crlf, prev)) != string::npos)
	{
		string first_line(http_request.substr(prev, next - prev));
		prev = next;
		stringstream sstream(first_line);
		sstream >> (phttphdr->method);
		sstream >> (phttphdr->url);
		sstream >> (phttphdr->version);
	}
	else
	{
		perror("tyhp_parse_http_request: http_request has not a \\r\\n");
		return false;
	}

	//查找"\r\n\r\n"的位置
	int pos_crlfcrlf = http_request.find(crlfcrlf, prev);
	if(pos_crlfcrlf == string::npos)
	{
		perror("tyhp_parse_http_request: http_request has not a \"\r\n\r\n\"");
		return false;
	}

	//解析首部行
	string buff, key, value;
	while(1)
	{
		next = http_request.find(crlf, prev+2);
		
		//如果找到的next不超过"\r\n\r\n"的位置
		if(next <= pos_crlfcrlf)
		{
			//buff保存了一行
			buff = http_request.substr(prev + 2, next - prev - 2);
			int end = 0;
			//跳过前置空白符，到达首部关键字的起始位置
			for(; isblank(buff[end]); ++end)
				;
			int beg = end;
			//到达首部关键字的结束位置
			for(; buff[end] != ':' && !isblank(buff[end]); ++end)
				;
			key = buff.substr(beg, end - beg);
			//跳至首部值的起始位置
			for(; (!isalpha(buff[end]) && !isdigit(buff[end])); ++end)
				;
			beg = end;
			//到达首部值的结束位置
			for(; next != end; ++end)
				;
			value = buff.substr(beg, end - beg);
			phttphdr->header.insert(make_tyhp_header(key, value));

			prev = next;
		}
		else
		{
			break;
		}
	}

	//获取http请求包的实体值（一般情况下不存在）
	phttphdr->body = http_request.substr(pos_crlfcrlf + 4, http_request.size() - pos_crlfcrlf - 4);

	return true;
}

/*
 *函数作用：根据key的值在phttphdr所指向的tyhp_http_header_t中查找相对应的值
 *函数参数：key为关键字，header
 *函数返回值: -返回空值表示查找失败，否则返回相应的值
 */
string tyhp_get_value_from_http_header(const string& key, const tyhp_header& header)
{
	if(header.empty())
		return "";
	tyhp_header::const_iterator cit = header.find(key);
	if(cit == header.end())
		return "";
	return (*cit).second;
}


/*int main(int argc, char const *argv[])
{
	tyhp_http_header_t *phttphdr = tyhp_alloc_http_header();
	string http_request(

"GET /home/zxh HTTP1.1\r\n\
Lengh: 8080\r\n\
Date: July Mon 2013\r\n\
\r\n\
<html>\n\
\thaha\n\
</html>"

);
	cout<<"http_request size:"<<http_request.size()<<endl;
	tyhp_parse_http_request(http_request, phttphdr);

	cout<<phttphdr->method<<" "<<phttphdr->url<<" "<<phttphdr->version<<endl;
	string str;
	str = tyhp_get_value_from_http_header("Lengh", phttphdr->header);
	cout<<"Lengh:"<<str<<endl;
	str = tyhp_get_value_from_http_header("Date", phttphdr->header);
	cout<<"Lengh:"<<str<<endl;
	cout<<phttphdr->body<<endl;

	tyhp_free_http_header(phttphdr);
	return 0;
}
*/
