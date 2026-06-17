#ifndef __FACTORY_HPP__
#define __FACTORY_HPP__


#include "singleton.hpp"
#include <functional>
#include <map>
#include <iostream>
#include <stdexcept>
#define TEMPLATES KEY, BASE, ARGS...
#define FACTORY_TEMPLATE template <class KEY, class BASE, class... ARGS>

FACTORY_TEMPLATE
class Factory
{
public:
    ~Factory() = default;
    Factory(Factory&) = delete;
    Factory &operator=(Factory&) = delete;
    void AddCtor(KEY key_, std::function<BASE(ARGS...)> func_);
    BASE Create(KEY key_, ARGS...args_);
    friend class Singleton<Factory<KEY, BASE, ARGS...>>;
private:
    Factory() = default;
    std::map<KEY, std::function<BASE(ARGS...)>> m_map;
};
FACTORY_TEMPLATE
void Factory<TEMPLATES>::AddCtor(KEY key_, std::function<BASE(ARGS...)> func_){m_map[key_] = func_;}
FACTORY_TEMPLATE
BASE Factory<TEMPLATES>::Create(KEY key_, ARGS... args_)
{
    
    if(m_map.find(key_) != m_map.end())
    {
        return m_map[key_](args_...);
    }
    else 
    {
        //std::cout << __PRETTY_FUNCTION__ << " key =  " << key_ << std::endl;
        throw std::out_of_range("key is not in map");
    }
};


#endif //__FACTORY_HPP__





