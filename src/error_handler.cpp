
#include <iostream>
#include <ostream>

#include "error_handler.hpp"


void CheckAndLog::NegativeCheck(const int& value, const std::string& message,const char* file, int line,const char* function) 
{
    #ifndef NDEBUG
    std::string log = CheckAndLog::PhraseLine(message,file,line,function);
    #endif // NDEBUG

    if (0 > value) 
    {
        #ifndef NDEBUG
        Singleton<Logger>::GetInstance()->Log(Logger::SEVERITY::INFO, log + " FAIL");
        #endif
        throw std::runtime_error(log + " FAIL");
    }
    #ifndef NDEBUG
    //std::cout << message << std::endl;
    // //std::cout<< "in negative: "<< Singleton<Logger>::GetInstance() << std::endl;
    Singleton<Logger>::GetInstance()->Log(Logger::SEVERITY::INFO, log + " SUCCESS");
    #endif
}


void CheckAndLog::JustLog(const std::string& message,const char* file , int line ,const char* function)
{
    std::string log = CheckAndLog::PhraseLine(message,file,line,function);
    Singleton<Logger>::GetInstance()->Log(Logger::SEVERITY::INFO, log);    
}

std::string CheckAndLog::PhraseLine(const std::string& message,const char* file, int line,const char* function)
{
    
    std::string log = "  ";
    log.append(file);
    log.append("           ");
    log.append(function);
    log.append("              ");
    log.append(std::to_string(line));
    log.append("    ");
    log.append(message);
    return log;

}