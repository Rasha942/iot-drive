

#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <fstream>
#include "minion.hpp"
int main(int argc, char* argv[])
{

    if(2 > argc)
    {
        std::cout << "Usage: minion.out minion_num(1-3)" <<std::endl;
    }
    std::cout << "minion num: " << argv[1] << std::endl;
    char* sock_port = (char*)UDP_PORT_MINION_1;
    if(0 != strcmp("1",argv[1]))
    {
        if(0 == strcmp("2",argv[1]))
        {
            sock_port = (char*)UDP_PORT_MINION_2;
        }
        else 
        {
            sock_port = (char*)UDP_PORT_MINION_3;
        }
    }
    std::vector<int> fds_vector(1);
    // UDPSocket udp_socket(UDP_PORT);
    // fds_vector[UDP_INDX] = udp_socket.GetSocketFD();
    std::shared_ptr<MasterProxy> proxy(Singleton<MasterProxy,const char*>::GetInstance(sock_port));
    fds_vector[0] = proxy->GetSocketFD();
    std::vector<std::tuple<int,Reactor::Mode,std::shared_ptr<IInputProxy>>> reactor_vector = {{fds_vector[0],Reactor::Mode::read,proxy}};
    std::vector<std::pair<int,std::function<std::shared_ptr<ICommand>()>>> factory_vector = {{MINION_READ_CMD_KEY,CreateMinionReadCmd},{MINION_WRITE_CMD_KEY,CreateMinionWriteCmd}};
    std::shared_ptr<Reactor::IListener> listener = std::make_shared<FDListen>();
    Framework fr(reactor_vector,factory_vector,"./plugins", listener);
    try {
        fr.Run();
    
    } catch (std::bad_function_call& error) 
    {
        
        std::cerr << error.what() << std::endl;
    }
}