/*********************************************
* 负责分发消息服务模块，其实就是把外部收到的消息，转化成内部事件，也就是data->msg->event的解码过程，
* 然后再把事件投递至线程池的任务队列，由线程池调用其process 方法对事件进行处理，最终调用每个event的handler方法
* 来处理event，此时每个event handler需要subscribe该event后才会被调用到.
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

    virtual BOOL open();         //启动线程池（初始化线程池）
    virtual void close();

    virtual void subscribe(u32 eid, iEventHandler* handler);     //订阅
    virtual void unsubscribe(u32 eid, iEventHandler* handler);   //取消订阅

    virtual i32 enqueue(iEvent* ev);    //把事件投递到线程池中进行处理
    
    /* 因为线程池是c代码，要调用c++类函数（参数列表默认加this指针），要使用static（默认不会加this指针） 或使用其他方法 */
    static void svc(void* argv);   //线程池回调函数

    virtual iEvent* process(const iEvent* ev);       //对具体的事件进行分发处理

    static DispatchMsgService* getInstance();        //单例

    iEvent* parseEvent(const char* message, u32 len, u32 eid);

    void handleAllResponseEvent(NetworkInterface* interface);

private:
    void AssembleHB(ConnectSession* cs, iEvent* ev, i32 eid);

protected:

    thread_pool_t* tp;          //线程池

    static DispatchMsgService* DMS_;

    typedef std::vector<iEventHandler*> T_EventHandlers;        //事件处理器
    typedef std::map<u32, T_EventHandlers> T_EventHandlersMap;  //事件ID对应的事件处理器容器
    T_EventHandlersMap subscribers_;

    bool svr_exit_;        //服务状态

    static std::queue<iEvent*> response_events;    //处理完请求的响应事件
    static pthread_mutex_t   queue_mutex;          //互斥锁
};


#endif // _BRK_SERVICE_DISPATCH_EVENT_SERVICE_H_