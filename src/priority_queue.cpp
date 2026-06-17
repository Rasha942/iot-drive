
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <ctime>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <queue>
#include <unistd.h>
using namespace std;
bool test = true;
template <typename ITEM, typename Q = queue<ITEM>>

class WaitableQueue
{
    public:
    WaitableQueue() = default;
    WaitableQueue(const WaitableQueue& other_ ) = delete;
    WaitableQueue& operator=(const WaitableQueue& other_) = delete;
    ~WaitableQueue() = default;
    void Push(const ITEM& item_); 
    int Pop(time_t timeout_, ITEM& ret_); // non-blocking
    void Pop(ITEM& ret_);
    bool IsEmpty() const;
    private:
    Q m_queue;    
    mutable timed_mutex m_mutex;
    condition_variable_any m_cd;
    
};

template <typename ITEM, typename Q>
void WaitableQueue<ITEM,Q>::Push(const ITEM& item_)
{
    {
        unique_lock<timed_mutex> lock(m_mutex);
        m_queue.push(item_);
        // sleep(5);

    }
    m_cd.notify_one();
}
template <typename ITEM, typename Q>    
int WaitableQueue<ITEM, Q>::Pop(time_t timeout_, ITEM& ret_)
{
    chrono::time_point<chrono::system_clock> time_limit = chrono::system_clock::now() + chrono::seconds(timeout_);
    unique_lock<timed_mutex> lock(m_mutex,defer_lock);
    if(false == lock.try_lock_until(time_limit))
    {
        return -2;
    }
    if(m_cd.wait_until(lock, time_limit ,[this](){return !m_queue.empty();}))
    {   
        ret_ = m_queue.front();
        m_queue.pop();
        return 0;
    }
    else 
    {
     return -1;
    }
        


}
template <typename ITEM, typename Q>      // stack.e();

void WaitableQueue<ITEM, Q>::Pop(ITEM& ret_)
{
    unique_lock<timed_mutex> lock(m_mutex);
    m_cd.wait(lock, [this](){return !m_queue.empty();});
    ret_ = m_queue.front();
    m_queue.pop();
    
}

template <typename ITEM, typename Q>    
bool WaitableQueue<ITEM, Q>::IsEmpty() const
{   
    unique_lock<timed_mutex> lock(m_mutex);
    return m_queue.empty();
}
/*************************** */

template<typename ITEM, typename Q = priority_queue<ITEM>>

class PQ: private Q
{
    public:
    const ITEM& front();
    using Q::pop;
    using Q::push;
    using Q::empty;
};

template<typename ITEM, typename Q>
const ITEM& PQ<ITEM,Q>::front()
{
     return Q::top();
}



WaitableQueue<int, PQ<int>> wq;


void* func1(void* args)
{
    (void)args;
    int x = 0;
    wq.Pop(x);
    cout << "pop/push test: 5 = success " << x << endl;
    pthread_exit(NULL);
}

void* func2(void* args)
{
    (void)args;
    int y = 5;
    wq.Push(y);
    pthread_exit(NULL);


}


void* func3(void* args)
{
    (void)args;
    int x = 0;
    wq.Pop(5, x);
    cout << "push/pop(timeout not reached) test: 5 = success " << x << endl;
    pthread_exit(NULL);
}
void* func4(void* args)
{
    (void)args;
    int x = 0;
    wq.Pop(5, x);
    cout << "push/pop(timeout reached) test: 0 = success " << x << endl;
    pthread_exit(NULL);
}
void* func5(void* args)
{
    (void)args;
    int x = 0;
    cout << "mutex timeout -2 = success: " << wq.Pop(0, x)<< endl;
    pthread_exit(NULL);
}
template<class F>
class A
{
    public:
    void foo(){printf("hi\n");}
};






