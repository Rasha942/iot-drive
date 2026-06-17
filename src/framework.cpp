
#include "dirmonitor.hpp"
#include "factory.hpp"
#include "input_mediator.hpp"
#include "reactor.hpp"
#include "singleton.hpp"
#include "framework.hpp"

#define FRAMEWORK_ARGS std::vector<std::tuple<int , Reactor::Mode, std::shared_ptr<IInputProxy>>> fdCallbacks \
    , std::vector<std::pair<int, std::function<std::shared_ptr<ICommand>()>>> crators \
    , const std::string& pluginDirPath, std::shared_ptr<Reactor::IListener> ilistener

Framework::Framework(FRAMEWORK_ARGS):m_reactor(Singleton<Reactor, Reactor::IListener* >::GetInstance(ilistener.get())),m_dirmonitor(Singleton<DirMonitor, std::string>::GetInstance(pluginDirPath))
{
    DLLLoader* dllLoader = Singleton<DLLLoader>::GetInstance();
    m_dirmonitor->DMRegister(dllLoader->GetCallback());
    // dllLoader->DLLInit();
    for(auto i: crators)
    {
        Singleton<Factory<int, std::shared_ptr<ICommand>>>::GetInstance()->AddCtor(i.first,i.second);
    }
    InputMediator(*m_reactor, fdCallbacks); 

}
void Framework::Run()
{
    m_reactor->Run(); 
}

