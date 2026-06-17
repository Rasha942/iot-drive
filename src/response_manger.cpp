#include "response_manger.hpp"



ThreadSafeUID ResponseManeger::RegisterCommand()
{
    
    ThreadSafeUID uid;
    u_int32_t key = uid.GetUniqueCounter();
    m_watchlist[key] = PROCESSING_REQUEST;
    return uid;
}
void ResponseManeger::RegisterResponse(const ThreadSafeUID& uid_)
{
    u_int32_t key = uid_.GetUniqueCounter();
    m_watchlist[key] = DONE;
}
bool ResponseManeger::HasResponseRecived(const ThreadSafeUID& uid_)
{
    u_int32_t key = uid_.GetUniqueCounter();
    if (m_watchlist[key])
    {   m_watchlist.erase(key);
        return true;
    }
    return false;
}