
#include "logger.hpp"
#include "threadpool.hpp"
#include "command.hpp"
#include "factory.hpp"
#include "ai.hpp"
using namespace std;
#define ADMIN (3)
#define ADMIN_STOP (4)

ThreadPool::ThreadPool(size_t num_threads):m_thread_arr(num_threads),m_thread_num(num_threads),m_resume(0),m_pause_ready(0)
{
    for(size_t i = 0; i < m_thread_num; ++i)
    {
        m_thread_arr[i] = thread(&ThreadPool::ThreadFunc,this);
    }
}
ThreadPool::~ThreadPool()
{   
    shared_ptr<AdminTPTask> kill_pill = make_shared<AdminTPTask>(&ThreadPool::Kill,*this);
    for(size_t i = 0; i < m_thread_num; ++i)
    {
        AdminAddTask(kill_pill,ADMIN_STOP);
    }
    for(size_t i = 0; i < m_thread_arr.size(); ++i)
    {
        if(true == m_thread_arr[i].joinable())
        m_thread_arr[i].join();
    }
}

void ThreadPool::AddTask(shared_ptr<ITPTask> task,Priority priority)
{
    AdminAddTask(task,priority);
}
void ThreadPool::AdminAddTask(shared_ptr<ITPTask> task, int priority)
{
    pair<shared_ptr<ITPTask>, int> pair(task,priority);
    m_tasks_pq.Push(pair);
}

void ThreadPool::ThreadFunc()
{
    pair<shared_ptr<ITPTask>,int> pair;  
    while(pair.second != ADMIN_STOP)
    {
        if(0 == m_tasks_pq.Pop(1,pair))
        pair.first->Run();
    }
}
void ThreadPool::PauseTask()
{
    unique_lock<mutex>lock(m_mutex);
    ++m_pause_ready;
    m_cd.wait(lock, [this](){return m_resume == true;});
    --m_pause_ready;
}
void ThreadPool::Pause()
{
    while(m_pause_ready != 0){;}
    m_resume = false;
    shared_ptr<AdminTPTask> pause_task = make_shared<AdminTPTask>(&ThreadPool::PauseTask,*this);
    for(size_t i = 0; i < m_thread_num; ++i)
    {
        AdminAddTask(pause_task,ADMIN);
    }
    while(m_pause_ready < m_thread_num){;}   
}
void ThreadPool::Resume()
{
    m_resume = true;
    m_cd.notify_all();
}

void ThreadPool::Kill(){}

void ThreadPool::SetNumThreads(size_t num)
{
        for(size_t i = m_thread_num; i < num; ++i)
        {
            m_thread_arr.push_back(thread(&ThreadPool::ThreadFunc,this));
        } 
        shared_ptr<AdminTPTask> kill_pill = make_shared<AdminTPTask>(&ThreadPool::Kill,*this);
        for(size_t i = m_thread_num; i > num; --i)
        {
            AdminAddTask(kill_pill,ADMIN_STOP);
        }
    
    m_thread_num = num;
}
/*************TASKS************** */
ThreadPool::ITPTask::~ITPTask(){}
ThreadPool::FunctionTPTask::FunctionTPTask(function<void()> func):m_func(func){}
void ThreadPool::FunctionTPTask::Run()
{
    m_func();
}
ThreadPool::FunctionTPTask::~FunctionTPTask(){}
ThreadPool::LogTask::LogTask(std::string str):m_str(str){}
ThreadPool::LogTask::~LogTask(){}
void ThreadPool::LogTask::Run()
{
    Logger* logger = Singleton<Logger>::GetInstance();
    logger->Log(Logger::SEVERITY::INFO,m_str);
    
}

TPTask::TPTask(std::shared_ptr<IArgsKey> taskargs):m_taskArgs(taskargs){}

void TPTask::Run()  
{
    int factory_key = m_taskArgs->GetKey();
    std::shared_ptr<ICommand> cmd;
    
    cmd = Singleton<Factory<int, std::shared_ptr<ICommand>>>::GetInstance()->Create(factory_key);
        
    std::pair<std::function<bool()>, std::chrono::milliseconds> res = cmd->Execute(m_taskArgs); 
    if(res.first != nullptr)
    {
        new AI(res.first, res.second);
    }
}

