#ifndef __SCHEDULER_HPP__
#define __SCHEDULER_HPP__
#include <bits/types/sigevent_t.h>
#include <chrono>
#include <memory>
#include <ctime>
#include <mutex>

#include "priority_queue.hpp"
#include "singleton.hpp"    
#include "logger.hpp"

#define B_SEC_BIG(a ,b) (a).second.it_value.tv_sec < (b).second.it_value.tv_sec
#define B_SEC_EQUAL(a ,b) (a).second.it_value.tv_sec == (b).second.it_value.tv_sec
#define B_NSEC_BIG(a ,b) (a).second.it_value.tv_nsec < (b).second.it_value.tv_nsec

class Scheduler
{
public:
    
    class ISchedTask
    {
    public:
        virtual ~ISchedTask() = default;
        virtual void Execute() = 0;
    };
    
    ~Scheduler();
    void AddTask(std::shared_ptr<ISchedTask> task, 
                    const std::chrono::milliseconds& miliSecDelta);

private:
    Scheduler(); // for Singleton
    friend class Singleton<Scheduler>;

    using Item = std::pair<std::shared_ptr<ISchedTask>, struct itimerspec>;

    struct Comparator
    {
        bool operator()(Item a, Item b)
        {
            if (B_SEC_BIG(a, b) || (B_SEC_EQUAL(a, b) && B_NSEC_BIG(a, b)))
            {
                return false;
            }
            return true;
        }
    };

    static void ThreadFunc(union sigval val);
    static void SetTime(Scheduler* schd);
    WaitableQueue <Item, PQ<Item, 
                    std::priority_queue<Item, std::vector<Item>, Comparator>>> m_wq;
    timer_t m_timerId;
    struct sigevent m_sigEvent;
    Logger* m_logger;
    Item m_currentTask;
    std::mutex m_mutex;
};

#endif //__ILRD_SCHEDULER_HPP__
