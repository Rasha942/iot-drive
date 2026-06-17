#ifndef __ILRD_RD_FRAMEWORK_HPP__
#define __ILRD_RD_FRAMEWORK_HPP__

#include <memory>

#include "inputproxy.hpp"
#include "dirmonitor.hpp"
#include "reactor.hpp"
#include "command.hpp"
class Framework
{
public:
    Framework(std::vector<std::tuple<int , Reactor::Mode, std::shared_ptr<IInputProxy>>> fdCallbacks
    , std::vector<std::pair<int, std::function<std::shared_ptr<ICommand>()>>> crators
    , const std::string& pluginDirPath, std::shared_ptr<Reactor::IListener> ilistener); // we can also choose to accept IListener from user for reactor, but we can also decide not
                                         // to and choose it for ourselfs
    void Run();
  

private:
    Reactor* m_reactor;
    DirMonitor* m_dirmonitor;
};


#endif // __ILRD_RD_FRAMEWORK_HPP__