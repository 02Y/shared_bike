#include "NetworkInterface.h"
#include "DispatchMsgService.h"
#include "bike.pb.h"

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//切记：ConnectSession 必须是C类型的成员变量（例：memset会把string类对象内部打乱）
static ConnectSession* session_init(int fd, struct bufferevent* bev) {
	ConnectSession* temp = nullptr;
	temp = new ConnectSession();

	if (!temp) {
		fprintf(stderr, "malloc failed. reason: %m\n");
		return nullptr;
	}

	memset(temp, '\0', sizeof(ConnectSession));
	temp->bev = bev;
	temp->fd = fd;

	return temp;
}

void session_free(ConnectSession* cs)
{
	if (cs)
	{
		if (cs->read_buf)
		{
			delete[] cs->read_buf;
			cs->read_buf = nullptr;

		}

		if (cs->write_buf)
		{
			delete[] cs->write_buf;
			cs->write_buf = nullptr;
		}
		delete cs;
	}
}

void session_reset(ConnectSession* cs)
{
	if (cs)
	{
		if (cs->request)
		{
			delete cs->request;
			cs->request = nullptr;
		}

		if (cs->response)
		{
			delete cs->response;
			cs->response = nullptr;
		}

		if (cs->read_buf)
		{
			delete[] cs->read_buf;
			cs->read_buf = nullptr;
		}

		if (cs->write_buf)
		{
			delete[] cs->write_buf;
			cs->write_buf = nullptr;
		}

		cs->session_stat = SESSION_STATUS::SS_REQUEST;     //处理请求状态
		cs->req_stat = MESSAGE_STATUS::MS_READ_HEADER;     //读头部

		cs->message_len = 0;
		cs->read_message_len = 0;
		cs->read_header_len = 0;
		cs->sent_len = 0;
	}
}

NetworkInterface::NetworkInterface()
{
	base_ = nullptr;
	listener_ = nullptr;
}

NetworkInterface::~NetworkInterface()
{
	close();
}

bool NetworkInterface::start(int port)
{
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);

	base_ = event_base_new();
	listener_ = evconnlistener_new_bind(base_, NetworkInterface::listener_cb, base_, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, 
									512, (struct sockaddr*)&sin, sizeof(struct sockaddr_in));

	return listener_ ? true : false;
}

void NetworkInterface::close()
{
	if (base_)
	{
		event_base_free(base_);
		base_ = nullptr;
	}

	if (listener_)
	{
		evconnlistener_free(listener_);
		listener_ = nullptr;
	}
}

void NetworkInterface::listener_cb(evconnlistener* listener, evutil_socket_t fd, sockaddr* sock, int socklen, void* arg)
{
	struct event_base* base = (struct event_base*)arg;
	LOG_DEBUG("accept a client %d\n", fd);

	//为这个客户端分配一个bufferevent  
	struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

	ConnectSession* cs = session_init(fd, bev);
	cs->session_stat = SESSION_STATUS::SS_REQUEST;     //处理请求状态
	cs->req_stat = MESSAGE_STATUS::MS_READ_HEADER;     //读头部

	strcpy(cs->remote_ip, inet_ntoa(((sockaddr_in*)sock)->sin_addr));
	LOG_DEBUG("remote ip : %s\n", cs->remote_ip);

	bufferevent_setcb(bev, handle_request, handle_response, handle_error, cs);   //超时会调用出错的回调
	bufferevent_enable(bev, EV_READ | EV_PERSIST);

	timeval t1 = { 10, 0 };   //10s 0微妙
	bufferevent_set_timeouts(bev, &t1, &t1);  //读写事件都设定为10秒超时   /* 超时值应设置在配置文件 */
}

/*****************************************************
*            4字节         2个字节         4个字节
*  请求格式：FBEB	       事件ID	      数据长度N	    数据内容

*  1.包标识：  包头部的特殊标识, 用来标识包的开始
*  2.事件类型：事件ID, 固定两个字节表示
*  3.数据长度：数据包的大小, 固定长度4字节。
*  4.数据内容：数据内容, 长度为数据头定义的长度大小。
*
* ****************************************************/
void NetworkInterface::handle_request(bufferevent* bev, void* arg)
{
	ConnectSession* cs = (ConnectSession*)arg;
	
	if (cs->session_stat != SESSION_STATUS::SS_REQUEST)      //正在处理请求
	{
		LOG_WARN("NetworkInterface::handle_request - wrong session state[%d].", cs->session_stat);
		return;
	}

	if (cs->req_stat == MESSAGE_STATUS::MS_READ_HEADER)
	{
		i32 len = bufferevent_read(bev, cs->header + cs->read_header_len, MESSAGE_HEADER_LEN - cs->read_header_len);
		cs->read_header_len += len;

		cs->header[cs->read_header_len] = '\0';
		LOG_DEBUG("recv from client<<<< %s\n", cs->header);

		if (MESSAGE_HEADER_LEN == cs->read_header_len)
		{
			if (0 == strncmp(cs->header, MESSAGE_HEADER_ID, strlen(MESSAGE_HEADER_ID)))
			{
				cs->eid = *((u16*)(cs->header + 4));
				cs->message_len = *((i32*)(cs->header + 6));

				LOG_DEBUG("NetworkInterface::handle_request - read  %d bytes in header, message len: %d\n", cs->read_header_len, cs->message_len);

				if (cs->message_len < 1 || cs->message_len > MAX_MESSAGE_LEN)       //防止消息超大或过小
				{
					LOG_ERROR("NetworkInterface::handle_request wrong message, len: %u\n", cs->message_len);
					bufferevent_free(bev);
					session_free(cs);
					return;
				}

				cs->read_buf = new char[cs->message_len];
				cs->req_stat = MESSAGE_STATUS::MS_READ_MESSAGE;    
				cs->read_message_len = 0;	
			}
			else
			{
				//直接关闭请求，不给予任何响应,防止客户端恶意试探
				LOG_ERROR("NetworkInterface::handle_request - Invalid request from %s\n", cs->remote_ip); 
				bufferevent_free(bev);
				session_free(cs);
				return;
			}
		}
	}

	if (cs->req_stat == MESSAGE_STATUS::MS_READ_MESSAGE && evbuffer_get_length(bufferevent_get_input(bev)) > 0/*拿到缓存里还有多少数据未读取*/)
	{
		i32 len = bufferevent_read(bev, cs->read_buf + cs->read_message_len, cs->message_len - cs->read_message_len);
		cs->read_message_len += len;

		LOG_DEBUG("NetworkInterface::handle_request - bufferevent_read: %d bytes, message len: %d read len: %d\n", 
			len, cs->message_len, cs->read_message_len);


		if (cs->read_message_len == cs->message_len)
		{
			cs->session_stat = SESSION_STATUS::SS_RESPONSE;
			iEvent* ev = DispatchMsgService::getInstance()->parseEvent(cs->read_buf, cs->read_message_len, cs->eid);    //解析事件

			delete[] cs->read_buf;
			cs->read_buf = nullptr;
			cs->read_message_len = 0;

			if (ev)     //解析完毕
			{
				cs->request = ev;
				ev->set_args(cs);     //方便写入响应信息
				DispatchMsgService::getInstance()->enqueue(ev);    //放入线程池
			}
			else       //解析出错
			{
				//直接关闭请求，不给予任何响应,防止客户端恶意试探
				LOG_ERROR("NetworkInterface::handle_request - ev is null. remote ip: %s, eid: %d\n", cs->remote_ip, cs->eid);
				bufferevent_free(bev);
				session_free(cs);
				return;
			}
		}
	}
}

void NetworkInterface::handle_response(bufferevent* bev, void* arg)
{
	LOG_DEBUG("NetworkInterface::handle_response ...\n");
}

//超时、连接关闭、读写出错等异常指定的回调函数
void NetworkInterface::handle_error(bufferevent* bev, short event, void* arg)
{
	ConnectSession* cs = (ConnectSession*)arg;
	LOG_DEBUG("NetworkInterface::handle_error ...\n");

	//连接关闭
	if (event & BEV_EVENT_EOF)
		LOG_DEBUG("connection closed\n");
	else if ((event & BEV_EVENT_TIMEOUT) && (event & BEV_EVENT_READING))
		LOG_WARN("NetworkInterface::reading timeout...[ip: %s]\n", cs->remote_ip);
	else if ((event & BEV_EVENT_TIMEOUT) && (event & BEV_EVENT_WRITING))
		LOG_WARN("NetworkInterface::writing timeout...[ip: %s]\n", cs->remote_ip);
	else if (event & BEV_ERROR)
		LOG_ERROR("NetworkInterface::some other error...[ip: %s]\n", cs->remote_ip);

	bufferevent_free(bev);
	session_free(cs);
}

void NetworkInterface::network_event_dispatch()
{
	event_base_loop(base_, EVLOOP_NONBLOCK);     //因为只能处理读的事件，所以不能阻塞
	DispatchMsgService::getInstance()->handleAllResponseEvent(this);    //处理所有的响应事件
}

void NetworkInterface::send_response_message(ConnectSession* cs)
{
	if (!cs->response)      //响应为空
	{
		bufferevent_free(cs->bev);
		if (cs->request)
		{
			delete cs->request;
		}

		session_free(cs);
	}
	else     //发送响应内容
	{
		bufferevent_write(cs->bev, cs->write_buf, cs->message_len + MESSAGE_HEADER_LEN);

		session_reset(cs);
	}
}


