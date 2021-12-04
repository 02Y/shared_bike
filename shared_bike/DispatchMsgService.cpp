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
	thread_mutex_create(&queue_mutex);    //����������
	tp = thread_pool_init();     //��ʼ���̳߳�

	return tp ? TRUE : FALSE;
}

void DispatchMsgService::close()
{
	svr_exit_ = TRUE;

	thread_pool_destroy(tp);     //�����̳߳�
	thread_mutex_destroy(&queue_mutex);     //���ٻ�����
	subscribers_.clear();

	tp = NULL;
}

void DispatchMsgService::subscribe(u32 eid, iEventHandler* handler)
{
	LOG_DEBUG("DispatchMsgService::subscribe eid: %u\n", eid);
	T_EventHandlersMap::iterator iter = subscribers_.find(eid);
	if (iter != subscribers_.end())    //��Ϊ�գ����Դ�����¼�
	{
		/* һ���¼����Ա�����¼����������� */
		T_EventHandlers::iterator hdl_iter = std::find(iter->second.begin(), iter->second.end(), handler);    //������������û�и��¼��Ĵ�����
		if (hdl_iter == iter->second.end())
		{
			iter->second.push_back(handler);        //���û�У�ע�ᴦ����
		}
	}
	else        //�����Դ�����¼����趩���¼�
	{
		subscribers_[eid].push_back(handler);       
	}
}

void DispatchMsgService::unsubscribe(u32 eid, iEventHandler* handler)
{
	T_EventHandlersMap::iterator iter = subscribers_.find(eid);
	if (iter != subscribers_.end())    //�ҵ����¼������˶�
	{
		/* һ���¼����Ա�����¼����������� */
		T_EventHandlers::iterator hdl_iter = std::find(iter->second.begin(), iter->second.end(), handler);    //������������û�и��¼��Ĵ�����
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

void DispatchMsgService::svc(void* argv)     //���̵߳���
{
	DispatchMsgService* dms = DispatchMsgService::getInstance();
	iEvent* ev = (iEvent*)argv;
	if (!dms->svr_exit_)
	{
		LOG_DEBUG("DispatchMsgService::svc ...\n");
		iEvent* rsp = dms->process(ev);

		if (rsp)    
		{
			rsp->dump(std::cout);     //���Ե������λ��
			rsp->set_args(ev->get_args());
		}
		else
		{
			//������ֹ��Ӧ���¼�
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
	if (handlers == subscribers_.end())     //û�ж����¼�
	{
		LOG_WARN("DispatchMsgService : no any event handler subscribed %d", eid);
		return NULL;
	}

	iEvent* rsp = NULL;
	for (auto iter = handlers->second.begin(); iter != handlers->second.end(); iter++)
	{
		iEventHandler* handler = *iter;
		LOG_DEBUG("DispatchMsgService : get handler: %s\n", handler->get_name().c_str());     //�õ����ĸ��¼�������

		rsp = handler->handle(ev);     //����ж���¼�������������ڴ�й¶
	}

	return rsp;     //����ж���¼�������������һ���������Ƽ�ʹ��vector ��list ���ض��rsp��
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

	cs->response = ev;     //������Ӧ�¼�

	cs->message_len = ev->ByteSize();
	cs->write_buf = new char[cs->message_len + MESSAGE_HEADER_LEN];

	//��װͷ��
	memcpy(cs->write_buf, MESSAGE_HEADER_ID, 4);
	*(u16*)(cs->write_buf + 4) = eid;
	*(i32*)(cs->write_buf + 6) = cs->message_len;

	//���л���������
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
			if (ev->get_eid() == EEVENTID_GET_MOBILE_CODE_RSP)   //��ȡ�ֻ���֤����Ӧ
			{
				ConnectSession* cs = (ConnectSession*)ev->get_args();
				AssembleHB(cs, ev, EEVENTID_GET_MOBILE_CODE_RSP);

				interface->send_response_message(cs);
			}
			else if(ev->get_eid() == EEVENTID_EXIT_RSP)         //�˳���Ӧ
			{
				ConnectSession* cs = (ConnectSession*)ev->get_args();
				cs->response = ev;     //������Ӧ�¼�
				interface->send_response_message(cs);
			}
			else if (ev->get_eid() == EEVENTID_LOGIN_RSP)       //��½��Ӧ
			{
				ConnectSession* cs = (ConnectSession*)ev->get_args();
				AssembleHB(cs, ev, EEVENTID_LOGIN_RSP);

				interface->send_response_message(cs);
			}
			//else if...
		}
	}
}
