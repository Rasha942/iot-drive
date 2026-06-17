#ifndef __IRLD_INPUT_MEDIATOR_HPP__
#define __IRLD_INPUT_MEDIATOR_HPP__

#include "inputproxy.hpp"

class InputMediator
{
private:
    struct ReactorCallback
    {
        ReactorCallback(std::shared_ptr<IInputProxy> proxy); 
        void operator()(int fd, Reactor::Mode mode);
    
    private:
            std::shared_ptr<IInputProxy> m_proxy;
    };  
public: 
InputMediator(Reactor& reacotr,  std::vector<std::tuple<int , Reactor::Mode, std::shared_ptr<IInputProxy>>> fd_callback);
~InputMediator() = default;   
};

#endif //__IRLD_INPUT_MEDIATOR_HPP__