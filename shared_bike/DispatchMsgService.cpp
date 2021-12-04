#include "DispatchMsgService.h"
#include "NetworkInterface.h"
#include "bike.pb.h"
#include "events_def.h"
#include <algorithm>

DispatchMsgService* DispatchMsgService::DMS_ = nullptr;
std::queue<iEvent*> DispatchMsgService::response_events;
pthread_mutex_t   DispatchMsgService::queue_mutex;

DispatchMsgService::DispatchMsgService() 
	:tp(nullptr)
{
}

DispatchMsgService::~DispatchMsgService()
{
}

BOOL DispatchMsgService::open()
{	
	svr_exit_ = false;
	thread_mutex_create(&queue_mutex);    //创建互斥锁
	tp = thread_pool_init();     //初始化线程池

	return tp ? TRUE : FALSE;
}

void DispatchMsgService::close()
{
	svr_exit_ = TRUE;

	thread_pool_destroy(tp);     //销毁线程池
	thread_mutex_destroy(&queue_mutex);     //销毁互斥锁
	subscribers_.clear();

	tp = NULL;
}

void DispatchMsgService::subscribe(u32 eid, iEventHandler* handler)
{
	LOG_DEBUG("DispatchMsgService::subscribe eid: %u\n", eid);
	T_EventHandlersMap::iterator iter = subscribers_.find(eid);
	if (iter != subscribers_.end())    //不为空，可以处理该事件
	{
		/* 一个事件可以被多个事件处理器处理 */
		T_EventHandlers::iterator hdl_iter = std::find(iter->second.begin(), iter->second.end(), handler);    //查找容器中有没有该事件的处理器
		if (hdl_iter == iter->second.end())
		{
			iter->second.push_back(handler);        //如果没有，注册处理器
		}
	}
	else        //不可以处理该事件，需订阅事件
	{
		subscribers_[eid].push_back(handler);       
	}
}

void DispatchMsgService::unsubscribe(u32 eid, iEventHandler* handler)
{
	T_EventHandlersMap::iterator iter = subscribers_.find(eid);
	if (iter != subscribers_.end())    //找到该事件，需退订
	{
		/* 一个事件可以被多个事件处理器处理 */
		T_EventHandlers::iterator hdl_iter = std::find(iter->second.begin(), iter->second.end(), handler);    //查找容器中有没有该事件的处理器
		if (hdl_iter != iter->second.end())
		{
			iter->second.erase(hdl_iter);    
		}
	}
}

i32 DispatchMsgService::enqueue(iEvent* ev)
{
	if (NULL == ev)
	{
		return -1;
	}

	thread_task_t* task = thread_task_alloc(0);
	task->handler = DispatchMsgService::svc;
	task->ctx = ev;

	return thread_task_post(tp, task);  
}

void DispatchMsgService::svc(void* argv)     //由线程调度
{
	DispatchMsgService* dms = DispatchMsgService::getInstance();
	iEvent* ev = (iEvent*)argv;
	if (!dms->svr_exit_)
	{
		LOG_DEBUG("DispatchMsgService::svc ...\n");
		iEvent* rsp = dms->process(ev);

		if (rsp)    
		{
			rsp->dump(std::cout);     //可以调整输出位置
			rsp->set_args(ev->get_args());
		}
		else
		{
			//生成终止响应的事件
			rsp = new ExitRspEv();
			rsp->set_args(ev->get_args());
		}

		thread_mutex_lock(&queue_mutex);
		response_events.push(rsp);
		thread_mutex_unlock(&queue_mutex);
	}
}

iEvent* DispatchMsgService::process(const iEvent* ev)
{
	LOG_DEBUG("DispatchMsgService::process -ev: %p\n", ev);
	if (NULL == ev)
	{
		return NULL;
	}

	u32 eid = ev->get_eid();

	LOG_DEBUG("DispatchMsgService::process - eid: %u\n", eid);

	if (eid == EEVENTID_UNKOWN)
	{
		LOG_WARN("DispatchMsgService : unknow evend id %d", eid);
		return NULL;
	}

	T_EventHandlersMap::iterator handlers = subscribers_.find(eid);
	if (handlers == subscribers_.end())     //没有订阅事件
	{
		LOG_WARN("DispatchMsgService : no any event handler subscribed %d", eid);
		return NULL;
	}

	iEvent* rsp = NULL;
	for (auto iter = handlers->second.begin(); iter != handlers->second.end(); iter++)
	{
		iEventHandler* handler = *iter;
		LOG_DEBUG("DispatchMsgService : get handler: %s\n", handler->get_name().c_str());     //用的是哪个事件处理器

		rsp = handler->handle(ev);     //如果有多个事件处理器会造成内存泄露
	}

	return rsp;     //如果有多个事件处理器，返回一个不合理（推荐使用vector 或list 返回多个rsp）
}

DispatchMsgService* DispatchMsgService::getInstance()
{
	if (nullptr == DMS_)
	{
		DMS_ = new DispatchMsgService();
	}

	return DMS_;
}

iEvent* DispatchMsgService::parseEvent(const char* message, u32 len, u32 eid)
{
	if (!message)
	{
		LOG_ERROR("DispatchMsgService::parseEvent - message is null[eid: %d].\n", eid);
		return nullptr;
	}

	if (eid == EEVENTID_GET_MOBILE_CODE_REQ)
	{
		tutorial::mobile_request mr;
		if (mr.ParseFromArray(message, len))
		{
			MobileCodeReqEv* ev = new MobileCodeReqEv(mr.mobile());
			return ev;
		}
	}
	else if (eid == EEVENTID_LOGIN_REQ)
	{
		tutorial::login_request lr;
		if (lr.ParseFromArray(message, len))
		{
			LoginReqEv* ev = new LoginReqEv(lr.mobile(), lr.icode());
			return ev;
		}
	}

	return nullptr;
}

void DispatchMsgService::AssembleHB(ConnectSession* cs, iEvent* ev, i32 eid)
{
	LOG_DEBUG("DispatchMsgService::AssembleHB - eid: %s.\n", getEidToString(eid));

	cs->response = ev;     //设置响应事件

	cs->message_len = ev->ByteSize();
	cs->write_buf = new char[cs->message_len + MESSAGE_HEADER_LEN];

	//组装头部
	memcpy(cs->write_buf, MESSAGE_HEADER_ID, 4);
	*(u16*)(cs->write_buf + 4) = eid;
	*(i32*)(cs->write_buf + 6) = cs->message_len;

	//序列化请求数据
	ev->SerializeToArray(cs->write_buf + MESSAGE_HEADER_LEN, cs->message_len);
}

void DispatchMsgService::handleAllResponseEvent(NetworkInterface* interface)
{
	bool done = false;

	while (!done)
	{
		iEvent* ev = nullptr;
		thread_mutex_lock(&queue_mutex);
		if (!response_events.empty())
		{
			ev = response_events.front();
			response_events.pop();
		}
		else
		{
			done = true;
		}
		thread_mutex_unlock(&queue_mutex);

		if (!done)
		{
			if (ev->get_eid() == EEVENTID_GET_MOBILE_CODE_RSP)   //获取手机验证码响应
			{
				ConnectSession* cs = (ConnectSession*)ev->get_args();
				AssembleHB(cs, ev, EEVENTID_GET_MOBILE_CODE_RSP);

				interface->send_response_message(cs);
			}
			else if(ev->get_eid() == EEVENTID_EXIT_RSP)         //退出响应
			{
				ConnectSession* cs = (ConnectSession*)ev->get_args();
				cs->response = ev;     //设置响应事件
				interface->send_response_message(cs);
			}
			else if (ev->get_eid() == EEVENTID_LOGIN_RSP)       //登陆响应
			{
				ConnectSession* cs = (ConnectSession*)ev->get_args();
				AssembleHB(cs, ev, EEVENTID_LOGIN_RSP);

				interface->send_response_message(cs);
			}
			//else if...
		}
	}
}
