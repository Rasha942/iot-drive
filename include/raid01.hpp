#ifndef __IRLD_RAID01_HPP__
#define __IRLD_RAID01_HPP__

#include <vector>
#include <memory> //shared_ptr
#include "singleton.hpp"
#include "inputproxy.hpp"

#define UDP_PORT_MINION_1 "7070"
#define UDP_PORT_MINION_2 "7080"
#define UDP_PORT_MINION_3 "7090"
#define KB 1024
#define MB (1024*KB)
#define MEMORY (12*MB) 
#define MEMORY_BACKUP (12 * MB)
#define MINION_SIZE (4 * MB)


class Raid01
{
public:
std::vector<std::pair<std::shared_ptr<MinionProxy>, u_int64_t>> OffsetToProxy(u_int64_t offset_);
friend class Singleton<Raid01>;  
private:
    Raid01();
    std::vector<std::shared_ptr<MinionProxy>> m_minion_proxy_vector;
};

#endif //__IRLD_RAID01_HPP__