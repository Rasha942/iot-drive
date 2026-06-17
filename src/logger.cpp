#include <iomanip>

#include "logger.hpp"


Logger::Logger()
{
    time_t genral_time = time(nullptr);
    tm *local_time = localtime(&genral_time);
    std::ostringstream time_stream;
    time_stream << std::put_time(local_time, "%Y-%m-%d_%H-%M-%S"); 
    std::string file_name = "logs/final_project_log_" + time_stream.str() + ".txt";
    m_fout.open(file_name, std::ios::app);
    
    std::string headline = "Date           | Time    | Type   | File                      | Function                                                                                        | Line | Message";
    m_fout << headline << std::endl;
    
    PhraseLine("Log started",__FILE_NAME__, __LINE__,__PRETTY_FUNCTION__);    
    m_thread = std::thread(&Logger::LogThreadFunc, this);
}

Logger::~Logger()
{
    std::string stop = "Log End\n";
    std::shared_ptr<LogTask> out(new LogTask(Logger::SEVERITY::INFO, stop, time(NULL), true));
    m_wq.Push(out);
    m_thread.join();
    m_fout.close();
}
std::string Logger::PhraseLine(const std::string& message, const char* file, int line, const char* function)
{

    std::string str = "  ";
    str.append(__FILE_NAME__);
    str.append("                                ");
    str.append(__PRETTY_FUNCTION__);
    str.append("                   ");
    str.append(std::to_string(__LINE__));
    str.append("       ");
    str.append(message);
    return str;
    
}

void Logger::LogThreadFunc()
{
    std::shared_ptr<LogTask> out;
    do
    {
        m_wq.Pop(out);
        out->Run(m_fout);
    } while (!out->IsStop());
}

void Logger::Log(SEVERITY severity, const std::string &string_to_log)
{
    std::shared_ptr<LogTask> task(new LogTask(severity, string_to_log, time(NULL), false));
    m_wq.Push(task);
}

void Logger::LogTask::Run(std::ofstream &fout)
{
    char sever_arr[3][11] = {"INFO - ", "WARNING - ", "ERROR - "};
    tm *local_time = localtime(&m_time);
    std::string sever_str = sever_arr[static_cast<int>(severity)];
    fout << std::put_time(local_time, "%Y-%m-%d %H:%M:%S") << " - "<< sever_str << string_to_log << std::endl;
}

bool Logger::LogTask::IsStop()
{
    return stop;
}
