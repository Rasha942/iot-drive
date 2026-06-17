#ifndef __IRLD_RESPONSE_MANGER_HPP__
#define __IRLD_RESPONSE_MANGER_HPP__

#include <unordered_map>
#include "message.hpp"
#include "singleton.hpp"
class ResponseManeger
{
public:
    ThreadSafeUID RegisterCommand();
    void RegisterResponse(const ThreadSafeUID& uid_);
    bool HasResponseRecived(const ThreadSafeUID& uid_);
    enum Status{PROCESSING_REQUEST,DONE};
    friend class Singleton<ResponseManeger>;
private:
    ResponseManeger() = default;
    std::unordered_map<u_int32_t, Status> m_watchlist;  
};

#endif //__IRLD_RESPONSE_MANGER_HPP__