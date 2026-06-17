

#include "raid01.hpp"
#include <iostream>



Raid01::Raid01():m_minion_proxy_vector(3)
{
    m_minion_proxy_vector[0]= std::make_shared<MinionProxy>(MinionProxy(UDP_PORT_MINION_1,"127.0.1.1"));
    m_minion_proxy_vector[1]= std::make_shared<MinionProxy>(MinionProxy(UDP_PORT_MINION_2,"127.0.1.1"));
    m_minion_proxy_vector[2]= std::make_shared<MinionProxy>(MinionProxy(UDP_PORT_MINION_3,"127.0.1.1"));


}
std::vector<std::pair<std::shared_ptr<MinionProxy>, u_int64_t>> Raid01::OffsetToProxy(u_int64_t offset_)
{
    std::cout<< __PRETTY_FUNCTION__ <<" offset =  " << offset_ << std::endl;
    size_t minion_idx = offset_ / MINION_SIZE;
    std::cout << __PRETTY_FUNCTION__ <<" idx =  " << minion_idx << std::endl;
    size_t minion_offset = offset_ %  MINION_SIZE;
    std::cout << __PRETTY_FUNCTION__ <<" minion offset =  " <<minion_offset << std::endl;
    
   return std::vector<std::pair<std::shared_ptr<MinionProxy>, u_int64_t>>{std::make_pair(m_minion_proxy_vector[minion_idx],minion_offset),std::make_pair(m_minion_proxy_vector[(minion_idx + 1) % 3],minion_offset + MINION_SIZE)};
}  