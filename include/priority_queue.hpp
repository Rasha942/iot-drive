


#ifndef __PRIORITY_QUEUE_HPP__
#define __PRIORITY_QUEUE_HPP__
#include <chrono>
#include <condition_variable>
#include <ctime>
#include <mutex>
#include <queue>
template <typename ITEM, typename Q = std::queue<ITEM>>

class WaitableQueue
{
    public:
    WaitableQueue() = default;
    WaitableQueue(const WaitableQueue& other_ ) = delete;
    WaitableQueue& operator=(const WaitableQueue& other_) = delete;
    ~WaitableQueue() = default;
    void Push(const ITEM& item_); 
    int Pop(time_t timeout_, ITEM& ret_); // non-blocking
    void Pop(ITEM& ret_); // blocking
    bool IsEmpty() const;
    private:
    Q m_queue;    
    mutable std::timed_mutex m_mutex;
    std::condition_variable_any m_cd;
    
};

template <typename ITEM, typename Q>
void WaitableQueue<ITEM,Q>::Push(const ITEM& item_)
{
    {
        std::unique_lock<std::timed_mutex> lock(m_mutex);
        m_queue.push(item_);

    }
    m_cd.notify_one();
}
template <typename ITEM, typename Q>    
int WaitableQueue<ITEM, Q>::Pop(time_t timeout_, ITEM& ret_)
{
    std::chrono::time_point<std::chrono::system_clock> time_limit = std::chrono::system_clock::now() + std::chrono::seconds(timeout_);
    std::unique_lock<std::timed_mutex> lock(m_mutex,std::defer_lock);
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
    std::unique_lock<std::timed_mutex> lock(m_mutex);
    m_cd.wait(lock, [this](){return !m_queue.empty();});
    ret_ = m_queue.front();
    m_queue.pop();   
}

template <typename ITEM, typename Q>    
bool WaitableQueue<ITEM, Q>::IsEmpty() const
{   
    std::unique_lock<std::timed_mutex> lock(m_mutex);
    return m_queue.empty();
}
/*************************** */

template<typename ITEM, typename Q = std::priority_queue<ITEM>>

class PQ: private Q
{
    public:
    const ITEM& front(){ return Q::top();}
    using Q::pop;
    using Q::push;
    using Q::empty;
};

#endif