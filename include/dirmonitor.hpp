#ifndef __DIRMONITOR_HPP__
#define __DIRMONITOR_HPP__



#include <memory>
#include <mutex>
#include <string>
#include <sys/inotify.h>
#include <thread>
#include <dlfcn.h> /*dlopen, dlclose*/
#include <vector>
#include "singleton.hpp"
#include "dispatcher.hpp"
#define EVENT_SIZE  sizeof (struct inotify_event) 
#define EVENT_BUF_LEN   1024 * ( EVENT_SIZE + 16 )
class DirMonitor
{
public:
    ~DirMonitor();
    void DMRegister(ACallback<std::string>&);
    void DMUnregister(ACallback<std::string>&);
    const std::string& GetPath() const;
    friend class  Singleton<DirMonitor,std::string>;
private:
    DirMonitor(std::string dir_path_ = "./plugins");
    void Monitor();
    std::string m_path;
    int m_inotify_fd;
    int m_watcher;
    std::thread m_thread;
    std::mutex m_mutex;
    Dispatcher<std::string> m_dispatcher;
};

class DLLLoader
{
public:
    ~DLLLoader();
    void Load(const std::string& lib);
    Callback<DLLLoader, std::string>& GetCallback() const;
    friend class Singleton<DLLLoader>;
private:
    DLLLoader();
    std::vector<void*> m_dll_list;
    std::shared_ptr<Callback<DLLLoader,std::string>> m_callback_ptr;
};
#endif //__DIRMONITOR_HPP__