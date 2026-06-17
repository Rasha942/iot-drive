#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__
#define TPS
#include <cstring>
#include <fstream>
#include <thread>

#include "priority_queue.hpp"
#include "singleton.hpp"

class Logger
{
public:
    enum class SEVERITY
    {
        INFO,
        WARNING,
        ERROR,
    };

    void Log(SEVERITY severity, const std::string &string_to_log);
    std::string PhraseLine(const std::string& message,const char* file ="Unspecified", int line = 0,const char* function = "Unspecified");
private:
    class LogTask
    {
    public:
        LogTask(SEVERITY severity_, const std::string &log_,time_t t, bool stop_) :  string_to_log(log_), severity(severity_), m_time(t), stop(stop_) {}
        void Run(std::ofstream &fout);
        bool IsStop();

    private:
        const std::string string_to_log;
        SEVERITY severity;
        time_t m_time;
        bool stop;
    };

    std::thread m_thread;
    WaitableQueue<std::shared_ptr<LogTask>> m_wq;
    std::ofstream m_fout;

    Logger();
    ~Logger();
    void LogThreadFunc();

    friend class Singleton<Logger>;
};


#endif
