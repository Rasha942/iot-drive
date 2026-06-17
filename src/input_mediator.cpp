#include "input_mediator.hpp"
#include "threadpool.hpp"

InputMediator::InputMediator(Reactor& reacotr,  std::vector<std::tuple<int , Reactor::Mode, std::shared_ptr<IInputProxy>>> fd_callbacks)
{
    for(const auto& i : fd_callbacks)
    {
        reacotr.Register(std::get<0>(i), std::get<1>(i), ReactorCallback(std::get<2>(i)));
    }    
}

InputMediator::ReactorCallback::ReactorCallback(std::shared_ptr<IInputProxy> proxy):m_proxy(proxy){}

void InputMediator::ReactorCallback::operator()(int fd, Reactor::Mode mode)
{
    std::shared_ptr<IArgsKey> p = m_proxy->GetArgsKey(fd, mode);
    if(p) 
    {
        Singleton<ThreadPool>::GetInstance()->AddTask(std::make_shared<TPTask>(p));
    }
     
}
