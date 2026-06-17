
#ifndef __THREADPOOL_HPP__
#define __THREADPOOL_HPP__
#include <semaphore.h> /*semaphore ops*/
#include <functional> /*function*/
#include <string>
#include <thread> /*thread*/
#include "singleton.hpp"
#include "priority_queue.hpp"
#include "argskey.hpp"
#include "logger.hpp"
// using namespace std;
#define PQ_PAIR std::pair<std::shared_ptr<ITPTask>, int>

class ThreadPool
{
public:
    friend class Singleton<ThreadPool>;
    ~ThreadPool();
    ThreadPool(const ThreadPool& other_) = delete;
    ThreadPool& operator=(const ThreadPool& other_) = delete;
    enum Priority {low,normal,high};
    class ITPTask
    {
    public:
        virtual ~ITPTask() = 0;
        virtual void Run() = 0;
    };
    class LogTask: public ITPTask
    {
    public:
        LogTask(std::string str);
        virtual ~LogTask();
        virtual void Run() override;
    private:
        std::string m_str;
    };
    class FunctionTPTask: public ITPTask
    {
    public:
        FunctionTPTask(std::function<void()> func_);
        virtual ~FunctionTPTask();
        virtual void Run();
    private:
        std::function<void()> m_func;
    };
    template<class OUTPUT>
    class FutureFuncTPTask: public ITPTask
    {
    public:    
        FutureFuncTPTask(std::function<OUTPUT()> func);
        virtual ~FutureFuncTPTask();
        virtual void Run();
        virtual OUTPUT GetOutput();
    private:
        std::function<OUTPUT()> m_func;
        sem_t m_sem;
        OUTPUT output;     

    };
    void AddTask(std::shared_ptr<ITPTask> task, Priority priority = normal);
    // void Run() // for debugging
    // { 
    //     for(size_t i = 0; i < m_thread_num; ++i)
    //     {
    //         m_thread_arr[i] = thread(&ThreadPool::ThreadFunc,this);
    //         // m_thread_arr[i].detach();
    //     }
    // }
    void Pause();
    void Resume();
    void SetNumThreads(size_t num);

private:
    ThreadPool(size_t num_threads = std::thread::hardware_concurrency() );

    struct Compare
    {
        bool operator()(PQ_PAIR pair1,PQ_PAIR pair2){return pair1.second < pair2.second;}
    };
    std::vector<std::thread> m_thread_arr;
    WaitableQueue<std::pair<std::shared_ptr<ITPTask>,int>,PQ<PQ_PAIR,std::priority_queue<PQ_PAIR,std::vector<PQ_PAIR>,Compare>>> m_tasks_pq;
    size_t m_thread_num;
    bool m_resume;
    size_t m_pause_ready;
    std::mutex m_mutex;
    std::condition_variable m_cd;
    void ThreadFunc();
    void PauseTask();
    void Kill();
    void AdminAddTask(std::shared_ptr<ITPTask> task, int priority);
    class AdminTPTask: public ITPTask
    {
    public:
        AdminTPTask(std::function<void(ThreadPool&)> pause, ThreadPool& tp):m_tp(tp),m_func(pause){}
        void Run() override{m_func(m_tp);}
        virtual ~AdminTPTask(){};
    private:
        ThreadPool& m_tp;
        std::function<void(ThreadPool&)>m_func;
    };
};

template<class OUTPUT>
ThreadPool::FutureFuncTPTask<OUTPUT>::FutureFuncTPTask(std::function<OUTPUT()> func):m_func(func){sem_init(&m_sem, 0, 0);}
template<class OUTPUT>
ThreadPool::FutureFuncTPTask<OUTPUT>::~FutureFuncTPTask(){sem_destroy(&m_sem);}
template<class OUTPUT>
void ThreadPool::FutureFuncTPTask<OUTPUT>::Run(){output = m_func(); sem_post(&m_sem);}
template<class OUTPUT>
OUTPUT ThreadPool::FutureFuncTPTask<OUTPUT>::GetOutput(){sem_wait(&m_sem);return output;}

class TPTask : public ThreadPool::ITPTask
{
public:
    TPTask(std::shared_ptr<IArgsKey> taskargs);
    void Run() override;
    
private:
    std::shared_ptr<IArgsKey> m_taskArgs;
};



#endif //__THREADPOOL_HPP__
