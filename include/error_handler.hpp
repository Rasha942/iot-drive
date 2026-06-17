#ifndef __ERROR_HANDLER_HPP__
#define __ERROR_HANDLER_HPP__

#include <stdexcept>
#include <string>
#ifndef NDEBUG
#include "logger.hpp"  
#endif

class CheckAndLog
{
public:
    
    static void NegativeCheck(const int& value, const std::string& message,const char* file = "Not Specified", int line = 0,const char* function = "Not Specified"); 
    template<class T>
    static void EqualToCheck(const T value, const T check_value, const std::string& message,const char* file = "Not Specified", int line = 0,const char* function = "Not Specified");
    template<class T>
    static void NotEqualToCheck(const T value, const T check_value, const std::string& message,const char* file = "Not Specified", int line = 0,const char* function = "Not Specified");
    static void JustLog(const std::string& message,const char* file = "Not Specified", int line = 0,const char* function = "Not Specified");
private:
static std::string PhraseLine(const std::string& message,const char* file = "Not Specified", int line = 0,const char* function = "Not Specified");
};

template<class T>
void CheckAndLog::EqualToCheck(const T value, const T check_value, const std::string& message,const char* file,int line,const char* function) 
{
    
    #ifndef NDEBUG
    std::string log = CheckAndLog::PhraseLine(message,file,line,function);
    #endif // NDEBUG

    if (check_value == value) 
    {
        #ifndef NDEBUG
        Singleton<Logger>::GetInstance()->Log(Logger::SEVERITY::INFO, log + " FAIL");
        #endif
        throw std::runtime_error(log + " FAIL");
    }
    #ifndef NDEBUG
    Singleton<Logger>::GetInstance()->Log(Logger::SEVERITY::INFO, log + " SUCCESS");
    #endif
}
template<class T>
void CheckAndLog::NotEqualToCheck(const T value, const T check_value, const std::string& message,const char* file,int line,const char* function) 
{
    
    #ifndef NDEBUG
    std::string log = CheckAndLog::PhraseLine(message,file,line,function);
    #endif // NDEBUG

    if (check_value != value) 
    {
        #ifndef NDEBUG
        Singleton<Logger>::GetInstance()->Log(Logger::SEVERITY::INFO, log + " FAIL");
        #endif
        throw std::runtime_error(log + " FAIL");
    }
    #ifndef NDEBUG
    Singleton<Logger>::GetInstance()->Log(Logger::SEVERITY::INFO, log + " SUCCESS");
    #endif
}

#endif //__ERROR_HANDLER_HPP__