/*********************************************
* ����ַ���Ϣ����ģ�飬��ʵ���ǰ��ⲿ�յ�����Ϣ��ת�����ڲ��¼���Ҳ����data->msg->event�Ľ�����̣�
* Ȼ���ٰ��¼�Ͷ�����̳߳ص�������У����̳߳ص�����process �������¼����д������յ���ÿ��event��handler����
* ������event����ʱÿ��event handler��Ҫsubscribe��event��Żᱻ���õ�.
**********************************************/

#ifndef _BRK_SERVICE_DISPATCH_EVENT_SERVICE_H_
#define _BRK_SERVICE_DISPATCH_EVENT_SERVICE_H_

#include <map>
#include <vector>
#include <queue>
#include "ievent.h"
#include "eventtype.h"
#include "iEventHandler.h"
#include "threadpool/thread_pool.h"
#include "NetworkInterface.h"

class DispatchMsgService
{
protected:
    DispatchMsgService();     //
public:

    virtual ~DispatchMsgService();

    virtual BOOL open();         //�����̳߳أ���ʼ���̳߳أ�
    virtual void close();

    virtual void subscribe(u32 eid, iEventHandler* handler);     //����
    virtual void unsubscribe(u32 eid, iEventHandler* handler);   //ȡ������

    virtual i32 enqueue(iEvent* ev);    //���¼�Ͷ�ݵ��̳߳��н��д���
    
    /* ��Ϊ�̳߳���c���룬Ҫ����c++�ຯ���������б�Ĭ�ϼ�thisָ�룩��Ҫʹ��static��Ĭ�ϲ����thisָ�룩 ��ʹ���������� */
    static void svc(void* argv);   //�̳߳ػص�����

    virtual iEvent* process(const iEvent* ev);       //�Ծ�����¼����зַ�����

    static DispatchMsgService* getInstance();        //����

    iEvent* parseEvent(const char* message, u32 len, u32 eid);

    void handleAllResponseEvent(NetworkInterface* interface);

private:
    void AssembleHB(ConnectSession* cs, iEvent* ev, i32 eid);

protected:

    thread_pool_t* tp;          //�̳߳�

    static DispatchMsgService* DMS_;

    typedef std::vector<iEventHandler*> T_EventHandlers;        //�¼�������
    typedef std::map<u32, T_EventHandlers> T_EventHandlersMap;  //�¼�ID��Ӧ���¼�����������
    T_EventHandlersMap subscribers_;

    bool svr_exit_;        //����״̬

    static std::queue<iEvent*> response_events;    //�������������Ӧ�¼�
    static pthread_mutex_t   queue_mutex;          //������
};


#endif // _BRK_SERVICE_DISPATCH_EVENT_SERVICE_H_