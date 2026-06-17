#ifndef __REACTOR_HPP__
#define __REACTOR_HPP__
#include <functional>
#include <memory>
#include <sys/types.h>
#include <vector>
#include <netinet/in.h>

#include <unordered_map>
#include "singleton.hpp"

class Reactor
{
public:
    enum Mode{read,write}; 
private:
    using LISTEN_PAIR = std::pair<int, Reactor::Mode>;
    using Handler = std::function<void(int,Mode)>;
public:
    class IListener
    {
    public:
        IListener() = default;
        virtual ~IListener(){}
        virtual std::vector<std::pair<int, Reactor::Mode>> Listen(const std::vector<std::pair<int, Reactor::Mode>> &fd_vector_) = 0;
    };
    void Register(int fd_, Mode mode_, Handler handle_);
    void Unregister(int fd_, Mode mode_);
    void Run();
    void Stop();
    friend class Singleton<Reactor, IListener*>;
private:
    explicit Reactor(IListener* listener_);
    struct hash_pair
    {
        size_t operator()(const LISTEN_PAIR p) const
        {
            return std::hash<int>{}(p.first * (1 + (10 * p.second)));
        }
    };

    std::unordered_map<LISTEN_PAIR, std::function<void(int, Mode)>,hash_pair> m_map;
   std::shared_ptr<IListener> m_listener;
    bool m_isStopped;
};

/******************************************************
 *  Class: FD_LISTEN                                  *      
 *  Purpose:                                          *
 *  Parameters:                                       *
 *  Returns:                                          *
 ******************************************************/

class FDListen:public Reactor::IListener
{
public:
    ~FDListen() override;
    virtual std::vector<std::pair<int, Reactor::Mode>> Listen(const std::vector<std::pair<int, Reactor::Mode>> &fd_vector_) override;
private:
    int m_tcp_socket;
    int m_udp_socket;
    sockaddr_in m_address;
    
};




#endif 