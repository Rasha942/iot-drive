

#include "reactor.hpp"
#include <memory>
#include <sys/select.h>
#include <unistd.h>
#include <vector>


Reactor::Reactor(IListener* listener_):m_listener(listener_),m_isStopped(false)
{
}

void Reactor::Register(int fd_, Mode mode_, Handler handle_)
{
    LISTEN_PAIR pair(fd_, mode_);
    m_map[pair] = handle_;  
}
void Reactor::Unregister(int fd_, Mode mode_)
{
    LISTEN_PAIR pair(fd_, mode_);
    m_map.erase(pair);
}

void Reactor::Run()
{
    while(false == m_isStopped)
    {
        std::vector<LISTEN_PAIR> vector;
        for (const auto& [key,value] : m_map)
        {
            vector.push_back(key);
        }

        vector = m_listener->Listen(vector);
        for(const auto& i : vector)
        {
            auto iter = m_map.find(i);
            if(m_map.end() != iter)
            {
                iter->second(i.first,i.second);
            }
        }
    }   
}

void Reactor::Stop()
{
    m_isStopped = true;
}

FDListen::~FDListen(){}
std::vector<std::pair<int, Reactor::Mode>> FDListen::Listen(const std::vector<std::pair<int, Reactor::Mode>> &fd_vector_) 
{
    fd_set fd_r;
    fd_set fd_w;
    FD_ZERO(&fd_r);
    FD_ZERO(&fd_w);
    int fd_max = 0;

    for(auto it =  fd_vector_.begin(); it != fd_vector_.end() ; ++it)
    {
        if(it->second == 0)
        {
            FD_SET(it->first, &fd_r);
        }
        else
        {
            FD_SET(it->first, &fd_w);
        }

        if(it->first > fd_max)
        {
            fd_max = it->first; 
        }
    }
    // printf("before select\n");
    while(0 >= select(fd_max + 1, &fd_r, &fd_w, NULL, NULL))
    {
        
        // printf("timout select\n");
        return fd_vector_;
    }

        // printf("fd select\n");

    std::vector<std::pair<int, Reactor::Mode>> vector_ret;
    for(auto it =  fd_vector_.begin(); it != fd_vector_.end() ; ++it)
    {
        if(FD_ISSET(it->first, &fd_r))
        {
            //std::cout << "fd isset = " << it->first << std::endl;

            vector_ret.push_back(*it);
        }
        else if(FD_ISSET(it->first, &fd_w))
        {
            vector_ret.push_back(*it);
        }

    }

    return vector_ret;
    
}


/*********************************************/

// class SelectListener: public Reactor::IListener
// {
// public:
//     SelectListener() = default;
//     ~SelectListener(){}
//     std::vector<std::pair<int, Reactor::Mode>> Listen(const std::vector<std::pair<int, Reactor::Mode>> &fd_vector_) override;
// }; 

// std::vector<std::pair<int, Reactor::Mode>> SelectListener::Listen(const std::vector<std::pair<int, Reactor::Mode>> &fd_vector_)
// {
//     int max_fd = 0;
//     int dead_time = 0;
//     fd_set read;
//     fd_set write;
//     struct timeval tv;
//     char stdin_buffer[100] = {0};
//     std::vector<std::pair<int, Reactor::Mode>> ret_vector;
  
//         tv.tv_sec = 7;
//         tv.tv_usec = 0;
//         FD_ZERO(&read);
//         FD_ZERO(&write);
//         for (const auto& i:fd_vector_)
//         {
//             if(Reactor::read == i.second)
//             {
//                 FD_SET(i.first, &read);
//             }
//             else 
//             {
//                 FD_SET(i.first, &write);
//             }
//             max_fd = std::max(max_fd,i.first);     
//         }
//         if(0 < select(max_fd + 1, &read ,&write, NULL, &tv))
//         {
//             for(const auto& i : fd_vector_)
//             {
//                 if((0 < FD_ISSET(i.first,&read) && Reactor::read == i.second )||0 < FD_ISSET(i.first,&write) && Reactor::write == i.second )
//                 ret_vector.push_back(i);
//             }
//         }
//         return ret_vector;
// }

// void Print(int fd, Reactor::Mode mode_)
// {
//     char buffer[100] = {0};
//     read(fd,buffer, sizeof(buffer));
//     //std::cout << buffer << std::endl;
// }

// #include <thread>
// #include <iostream>
// #include <cstring>
// #include <unistd.h>


// SelectListener l_listen;
// Reactor* react = Singleton<Reactor, Reactor::IListener*>::GetInstance(&l_listen);
// int stop = 0;

// void TestRead(int fd, Reactor::Mode)
// {
//     char buffer[100];
//     int size = read(fd,buffer, sizeof(buffer) + 1);
//     buffer[size] = '\0';
//     if (!strncmp(buffer, "stop", 4))
//     {
//         printf("zzzzzzzzzzzz -in\n");
//         react.Stop();
//         stop = 1;
//     }
//     printf("%d, aaaaaaaaaaa - %s\n\n\n", strcmp(buffer, "stop"), buffer);
// }

// int main()
// {
// char buff[100];
//     react->Register(0, Reactor::read, TestRead);
//     std::thread t1(&Reactor::Run, &react);
//     for (int i = 0; i < 50 && !stop; i++)
//     {
//         printf("before in\n");
//         std::cin >> buff;
//         printf("after in\n");
//     }
//     t1.join();
//     return 0;
// }
