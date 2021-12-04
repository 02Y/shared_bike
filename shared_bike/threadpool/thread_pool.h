#ifndef _THREAD_POOL_H_INCLUDED_
#define _THREAD_POOL_H_INCLUDED_

#ifdef __cplusplus     //告诉编译器，这部分代码按C语言的格式进行编译，而不是C++的
extern "C" {
#endif /* __cplusplus */

#include "thread.h"

#define DEFAULT_THREADS_NUM 4             //线程数量
#define DEFAULT_QUEUE_NUM  65535          //任务数量


typedef unsigned long         atomic_uint_t;
typedef struct thread_task_s  thread_task_t;
typedef struct thread_pool_s  thread_pool_t;


struct thread_task_s {          //thread_task_t     任务
    thread_task_t       *next;
    uint_t               id;
    void                *ctx;      //上下文（任务要带的参数）
    void               (*handler)(void *data);       //任务的函数
};

typedef struct {
    thread_task_t        *first;
    thread_task_t        **last;
} thread_pool_queue_t;          //任务队列

#define thread_pool_queue_init(q)                                         \
    (q)->first = NULL;                                                    \
    (q)->last = &(q)->first


struct thread_pool_s {         //thread_pool_t 
    pthread_mutex_t        mtx;        //互斥锁
    thread_pool_queue_t   queue;       //任务队列
    int_t                 waiting;
    pthread_cond_t         cond;       //条件变量

    char                  *name;
    uint_t                threads;
    int_t                 max_queue;
};

thread_task_t *thread_task_alloc(size_t size);
void thread_task_free(thread_task_t* task);
int_t thread_task_post(thread_pool_t *tp, thread_task_t *task);
thread_pool_t* thread_pool_init();
void thread_pool_destroy(thread_pool_t *tp);

#ifdef __cplusplus
}
#endif    //__cplusplus

#endif /* _THREAD_POOL_H_INCLUDED_ */
