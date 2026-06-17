#include <csignal>
#include <cstdio>
#include "error_handler.hpp"
// #include "logger.hpp"
#include "scheduler.hpp"
#include "string_manger.hpp"
#define SUCCESS 0


Scheduler::Scheduler(): m_wq(), m_logger(Singleton<Logger>::GetInstance())
{
    memset(&m_sigEvent, 0, sizeof(m_sigEvent));

    m_sigEvent.sigev_notify = SIGEV_THREAD;
    m_sigEvent.sigev_notify_function = Scheduler::ThreadFunc;
    m_sigEvent.sigev_value.sival_ptr = this;

    if (SUCCESS == timer_create(CLOCK_REALTIME,&m_sigEvent, &m_timerId)) 
    {
        CheckAndLog::JustLog(SCHEDULER_STRINGS::TIMER_CREATE_SECCES);
        m_logger->Log(Logger::SEVERITY::INFO, SCHEDULER_STRINGS::TIMER_CREATE_SECCES);
    }
    else 
    {
        m_logger->Log(Logger::SEVERITY::ERROR, SCHEDULER_STRINGS::TIMER_CREATE_FAIL);
        throw std::runtime_error(SCHEDULER_STRINGS::TIMER_CREATE_FAIL);
    }

    m_currentTask.first = nullptr;
}

Scheduler::~Scheduler()
{
    timer_delete(m_timerId);
}


void Scheduler::ThreadFunc(union sigval val)
{
    Scheduler* schd = reinterpret_cast<Scheduler*>(val.sival_ptr);
    std::unique_lock<std::mutex> lock(schd->m_mutex);

    if (nullptr != schd->m_currentTask.first) 
    {
        Item tempCurrentTask = schd->m_currentTask;
        if (!schd->m_wq.IsEmpty()) 
        {
            schd->m_wq.Pop(schd->m_currentTask);
            SetTime(schd);
        }
        else 
        {
            schd->m_currentTask.first = nullptr;
        }

        schd->m_mutex.unlock();
        tempCurrentTask.first->Execute();
        schd->m_mutex.lock();
    }
}

void Scheduler::SetTime(Scheduler* schd)
{
    if (SUCCESS == timer_settime(schd->m_timerId, TIMER_ABSTIME, &schd->m_currentTask.second, NULL))
    {
        schd->m_logger->Log(Logger::SEVERITY::INFO, SCHEDULER_STRINGS::TIMER_SETTIME_SECCES);
    }
    else 
    {
        schd->m_logger->Log(Logger::SEVERITY::ERROR, SCHEDULER_STRINGS::TIMER_SETTIME_FAIL);
        throw std::runtime_error(SCHEDULER_STRINGS::TIMER_CREATE_FAIL);
    }
}


void Scheduler::AddTask(std::shared_ptr<ISchedTask> task, const std::chrono::milliseconds& miliSecDelta)
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    long long totalMilliseconds = milliseconds + miliSecDelta.count();

    struct itimerspec itImerspec = {{0, 0}, 
                                    {totalMilliseconds / 1000, 
                                    (long int)(totalMilliseconds % 1000) * 1000000}};

    Item newItem = make_pair(task, itImerspec);

    std::unique_lock<std::mutex> lock(m_mutex);
    if (nullptr == m_currentTask.first) 
    {
        m_logger->Log(Logger::SEVERITY::INFO, "queue is empty");
        m_currentTask = newItem;
    }
    else if(B_SEC_BIG(m_currentTask, newItem) || (B_SEC_EQUAL(m_currentTask, newItem) && B_NSEC_BIG(m_currentTask, newItem)))
    {
        m_logger->Log(Logger::SEVERITY::INFO, "added to queue");
        m_wq.Push(newItem);
    }
    else 
    {
        m_logger->Log(Logger::SEVERITY::INFO, "\n    m_wq.Push(m_currentTask);\n    m_currentTask = newItem;");
        m_wq.Push(m_currentTask);
        m_currentTask = newItem;
    }

    lock.unlock();
    SetTime(this);
}
