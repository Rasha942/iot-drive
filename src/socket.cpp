
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include "socket.hpp"
#include "raz_nbd.hpp"
#include "error_handler.hpp"

#define MAX_BACKLOG 10

std::unordered_map<int, addrinfo*> ASocket::m_open_sockets;
std::mutex ASocket::m_mutex;
ASocket::ASocket(__socket_type type_,const char* port_,const char* ip_, SOCKET_ROLE role,int socket_fd):m_socket_fd(socket_fd)
{
    if(type_ == SOCK_DGRAM)
    {
        m_socket_fd = socket(AF_INET, type_, 0);
        printf("udp socket = %d", m_socket_fd);
             int opt = 1;
        CheckAndLog::NegativeCheck(setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)),"setsocket ",__FILE_NAME__,__LINE__, __PRETTY_FUNCTION__);
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = htons(std::stol(port_));
            m_addr.sin_addr.s_addr = inet_addr(ip_);
        if(role == SRVR)
        {
            int x = bind(m_socket_fd, (struct sockaddr*)&m_addr, sizeof(m_addr));
            printf("bind res = %d\n", x);
            return;
        }
        else if(role == CLINT)
        {
            // printf("udp_port_ = %d\n",udp_port_);
            // printf("udpip2 = %s\n", udp_ip2);
            inet_pton(AF_INET,port_ , &m_addr.sin_addr.s_addr);
            return;
        }
    }
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    if(SRVR == role)
    {
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = type_;
        hints.ai_flags = AI_PASSIVE;
    }
    else 
    {
        hints.ai_family = AF_INET;
        hints.ai_socktype = type_;
    }
    getaddrinfo(ip_, port_, &hints, &m_addrinfo);
    
    for (; m_addrinfo != NULL; m_addrinfo = m_addrinfo->ai_next)
    {
            
        m_socket_fd = socket(m_addrinfo->ai_family, m_addrinfo->ai_socktype, m_addrinfo->ai_protocol);
        if (0 > m_socket_fd)
        {
            //std::cout << "socket error" << std::endl;
            continue;
        }
        int opt = 1;
        CheckAndLog::NegativeCheck(setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)),"setsocket ",__FILE_NAME__,__LINE__, __PRETTY_FUNCTION__);
        if(SRVR == role)
        {
            if(0 > bind(m_socket_fd,m_addrinfo->ai_addr,m_addrinfo->ai_addrlen))
            {
                //std::cout << "bind error, trying again..." << std::endl;
                close(m_socket_fd);
                continue;
            }
        }    
        // std::unique_lock<std::mutex> lock(m_mutex);
        // m_open_sockets[m_socket_fd] = m_addrinfo;
        break;
    }
        
        if(m_addrinfo == NULL)
        {
            //std::cout << __PRETTY_FUNCTION__ << "  error" << std::endl;
        }

          
        // CheckAndLog::EqualToCheck<addrinfo>(m_addrinfo, NULL, "socket and bind",__FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
}
    


























    // if(0 > m_socket_fd)
    // {
    //     m_socket_fd = socket(AF_INET,type_,0);
    //         CheckAndLog::NegativeCheck(m_socket_fd, "TCP socket creation ",__FILE_NAME__,__LINE__, __PRETTY_FUNCTION__);
    //     std::unique_lock<std::mutex> lock(m_mutex);
    //     m_open_sockets[m_socket_fd] = m_address;
    // }
    // else 
    // {
    //     std::unique_lock<std::mutex> lock(m_mutex);
    //     if (m_open_sockets.end() != m_open_sockets.find(socket_fd))
    //     {
    //         m_address = m_open_sockets[m_socket_fd];
    //         return;
    //     }
    //     CheckAndLog::NegativeCheck(-1, "provided socket search ");
    // }
        
    // int opt = 1;
    // CheckAndLog::NegativeCheck(setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)),"setsocket ",__FILE_NAME__,__LINE__, __PRETTY_FUNCTION__);

    // m_address.sin_family = AF_INET;
    // m_address.sin_addr.s_addr = bind_flag ? INADDR_ANY : inet_addr("127.0.0.1");
    // m_address.sin_port = htons(port_); 
  
    // if(true == bind_flag)
    // {
    //     CheckAndLog::NegativeCheck(bind(m_socket_fd, (struct sockaddr *)&m_address, sizeof(m_address)),"bind");
    // }

ASocket::~ASocket()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_open_sockets.end() != m_open_sockets.find(m_socket_fd))
    {
        m_open_sockets.erase(m_socket_fd);
        close(m_socket_fd);
    }
}

int ASocket::GetSocketFD() const
{
    return m_socket_fd;
}
UDPSocket::UDPSocket(const char* port_,const char* ip_ , SOCKET_ROLE role ,int socket_fd):ASocket(SOCK_DGRAM,port_,ip_,role,socket_fd)
{}

void UDPSocket::Send(const char* buffer,int size_to_send, int flags)
{
    socklen_t len = sizeof(*m_addrinfo->ai_addr);
    unsigned int bytes_sent = sendto(m_socket_fd, buffer, size_to_send, 0, (struct sockaddr*)&m_addr, sizeof(m_addr));    //std::cout <<"bytes sent = " << bytes_sent << std::endl; 
    // CheckAndLog::NegativeCheck(sendto(m_socket_fd,"ping", strlen("ping") + 1, flags, m_addrinfo->ai_addr,len),"sendto",__FILE_NAME__,__LINE__, __PRETTY_FUNCTION__);
}

void UDPSocket::Recive(void* buffer, int size_to_read, int flags)
{
    socklen_t server_addr_len = sizeof(other_addr);
    int bytes_recived = recvfrom(m_socket_fd,buffer, size_to_read, flags, (struct sockaddr*)&other_addr, &server_addr_len);
    //std::cout <<"bytes recv = " << bytes_recived << std::endl; 
    // int bytes_left = 0;
    // ioctl(m_socket_fd,FIONREAD,&bytes_left);
    // //std::cout <<"bytes left  = " << bytes_left << std::endl; 
    // int error = 0;
    // socklen_t len = sizeof(error);
    // if (getsockopt(m_socket_fd, SOL_SOCKET, SO_ERROR, &error, &len) == 0 && error != 0) {
    //     printf("Socket error: %s\n", strerror(error));
    // }
    // else {
    //      printf("Socket error: not found\n");

    // }
}

void UDPSocket::Response(const char* buffer, int size_to_write, int flags)
{
    socklen_t server_addr_len = sizeof(other_addr);
    sendto(m_socket_fd, "pong", strlen("pong"), 0,(sockaddr*)&other_addr, server_addr_len);

}
TCPSocket::TCPSocket(const char* port_,const char* ip_ , SOCKET_ROLE role ,int socket_fd):ASocket(SOCK_STREAM,port_,ip_,role,socket_fd),m_child_socket(-1),m_role(role)
{
   
}


void TCPSocket::TCPListen()
{
    CheckAndLog::NegativeCheck(listen(m_socket_fd,5),"TCP Listen",__FILE_NAME__,__LINE__, __PRETTY_FUNCTION__);
}



void TCPSocket::TCPConnect()
{
    connect(m_socket_fd,m_addrinfo->ai_addr,m_addrinfo->ai_addrlen);
    // CheckAndLog::NegativeCheck(connect(m_socket_fd,m_addrinfo->ai_addr,m_addrinfo->ai_addrlen),"TCP connect",__FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
}
void TCPSocket::Send(const char* buffer,int size_to_send, int flags)
{
    int byts_sent = send(m_socket_fd, buffer, size_to_send, flags);
    std::cout << __PRETTY_FUNCTION__ << "  bytes sent = " << byts_sent << std::endl;
    CheckAndLog::NegativeCheck(byts_sent,"TCP send",__FILE_NAME__,__LINE__, __PRETTY_FUNCTION__);  
}

int TCPSocket::GetSocketFD() const
{
    return m_socket_fd;
}

int TCPSocket::GetChildSocketFD() const
{
    return m_child_socket;
}

void TCPSocket::Recive(void* buffer, int size_to_read, int flags)
{
    int rec_socket = m_socket_fd;
    if(m_role == SRVR)
    {
        rec_socket = m_child_socket;
    }
    int bytes_read = recv(rec_socket,buffer,size_to_read,flags);
    std::cout << __PRETTY_FUNCTION__ << "  bytes recv = " << bytes_read << std::endl;
    CheckAndLog::NegativeCheck(bytes_read,"TCP read",__FILE_NAME__,__LINE__, __PRETTY_FUNCTION__);
}

void TCPSocket::Response(const char* buffer, int size_to_write, int flags)
{
    socklen_t server_addr_len = sizeof(other_addr);
    int bytes_sent = write(m_child_socket,buffer,size_to_write);
    //std::cout << __PRETTY_FUNCTION__ << "  bytes sent = " << bytes_sent << std::endl;


}

int TCPSocket::TCPAccept()
{
    socklen_t other_addr_len = sizeof(other_addr);
    m_child_socket = accept(m_socket_fd,(sockaddr*)&other_addr, &other_addr_len);
    CheckAndLog::NegativeCheck(m_child_socket,"TCP accept",__FILE_NAME__,__LINE__, __PRETTY_FUNCTION__);

    return m_child_socket;

}