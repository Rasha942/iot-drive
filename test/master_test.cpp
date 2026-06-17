    
#include "master.hpp"


#include <iostream>


#define MAX_BACKLOG (10)
#define TCP_INDX 0
#define UDP_INDX 1

int main()
{    
    system("sudo modprobe nbd");
    system("echo 4 | sudo tee /sys/block/nbd0/queue/max_sectors_kb");
    // TCPSocket tcp_socket(TCP_PORT);
    // fds_vector[TCP_INDX] = tcp_socket.TCPListen();
    // UDPSocket udp_socket(UDP_PORT_MASTER);
 
    // int x = 0;
    //std::cout << "press any key and Enter to continue" << std::endl; 
    // std::cin >> x;
    std::vector<int> fds_vector(2);
    std::shared_ptr<IInputProxy> nbd_proxy(Singleton<NBDProxy>::GetInstance());
    std::shared_ptr<IInputProxy> response_proxy(Singleton<ResponseProxy>::GetInstance());
    fds_vector[UDP_INDX] = response_proxy->GetSocketFD();
    fds_vector[TCP_INDX] = nbd_proxy->GetSocketFD();
    std::vector<std::tuple<int,Reactor::Mode,std::shared_ptr<IInputProxy>>> reactor_vector = {{fds_vector[TCP_INDX],Reactor::Mode::read,nbd_proxy},{fds_vector[UDP_INDX],Reactor::Mode::read,response_proxy}};
    std::vector<std::pair<int,std::function<std::shared_ptr<ICommand>()>>> factory_vector = {{READ_CMD_KEY,CreateReadCommand},{WRITE_CMD_KEY,CreateWriteCommand}};
    std::shared_ptr<Reactor::IListener> listener = std::make_shared<FDListen>();
    Framework fr(reactor_vector,factory_vector,"./plugins", listener);
    // fr.Run();
      try {
        fr.Run();
    
    } catch (std::exception& error) 
    {
        
        std::cerr << error.what() << std::endl;
    }
}