
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <stdexcept> /*runtime error*/
#include <string>
#include <sys/inotify.h>
#include <thread>
#include <dlfcn.h> /*dlopen, dlclose*/
#include <vector>

#include "dispatcher.hpp"
#include "singleton.hpp"
#include "dirmonitor.hpp"
#define EVENT_SIZE  sizeof (struct inotify_event) 
#define EVENT_BUF_LEN   1024 * ( EVENT_SIZE + 16 )

DirMonitor::DirMonitor(std::string dir_path_):m_path(dir_path_),m_inotify_fd(0),m_watcher(0),m_thread()
{
    m_inotify_fd = inotify_init();
    if(0 > m_inotify_fd)
    {   
        throw std::runtime_error("inotify_init error");
    }
    m_watcher = inotify_add_watch(m_inotify_fd,m_path.c_str(), IN_CREATE  | IN_MOVED_TO | IN_MODIFY | IN_ATTRIB) ;
    if(0 > m_inotify_fd)
    {   
        throw std::runtime_error("inotify_add error");
    }
    m_thread = std::thread(&DirMonitor::Monitor,this);

}
DirMonitor::~DirMonitor()
{
    char cmd[200] = {0};
    sprintf(cmd,"touch %s/%s",m_path.c_str(),"exit.txt");
    system(cmd);
    m_thread.join();
    if (m_watcher  > 0)
    {
        inotify_rm_watch(m_inotify_fd, m_watcher);
    }
    if (m_inotify_fd > 0)
    {
        close(m_inotify_fd);
    }
}
void DirMonitor::Monitor()
{
    while(1)
    {
        char event_buffer[EVENT_BUF_LEN] = {0};
        int len = read(m_inotify_fd,event_buffer, EVENT_BUF_LEN);
        if (0 > len)
        {
            throw std::runtime_error("read error");
        }
        int i = 0;
        while(i < len)
        {
            struct inotify_event* event = reinterpret_cast<struct inotify_event*>(&event_buffer[i]);
            if (0 != event->len)
            {
                if(0 != strcmp(event->name + strlen(event->name) -3 ,".so")) 
                {             


                    if(0 == strcmp(event->name, "exit.txt") && (event->mask & IN_ATTRIB))
                    {
                        return;   
                    }
                    i += EVENT_SIZE + event->len;
                    continue;
                }
                if(event->mask & IN_CREATE  || /* event->mask & IN_MODIFY ||  */ event->mask & IN_MOVED_TO)
                { 
                    std::string str = m_path + '/' + event->name;
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_dispatcher.Notify(str);
                    //std::cout << "here2" << std::endl;

                }
            }
            i += EVENT_SIZE + event->len;

        }

    }
    
}


void DirMonitor::DMRegister(ACallback<std::string>& callback)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_dispatcher.Register(&callback);
}

void DirMonitor::DMUnregister(ACallback<std::string>& callback)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_dispatcher.Unregister(&callback);
}
const std::string& DirMonitor::GetPath() const
{
    return m_path;
}


DLLLoader::DLLLoader():m_callback_ptr(new Callback<DLLLoader, std::string>(*this,&DLLLoader::Load))
{
    const std::string& dir_path = Singleton<DirMonitor, std::string>::GetInstance("e")->GetPath();
    std::filesystem::directory_iterator dir_iter(dir_path);
    for(auto i : dir_iter)
    {
        if(i.is_regular_file() && ".so" == i.path().extension())
        {
            Load(i.path());
        }
    }

}
DLLLoader::~DLLLoader()
{
    for (auto i : m_dll_list)
    {
        dlclose(i);
    }
}

Callback<DLLLoader, std::string>& DLLLoader::GetCallback() const
{
    return *m_callback_ptr;
}
void DLLLoader::Load(const std::string& lib)
{
    //std::cout << lib.c_str()<< std::endl;
    void* ptr = dlopen(lib.c_str(), RTLD_LAZY);
    if(NULL == ptr)
    {
        //std::cout << "load error" << std::endl;
 
    }
    m_dll_list.push_back(ptr);

    // foo f1  = (foo)dlsym(ptr,"_Z3foov");
    // f1();
    // for (auto i : m_dll_list)
    // {
    //     if (i == NULL)
    //     {
    //         //std::cout << "load error" << std::endl;
    //     }
    // }

}
