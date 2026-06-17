#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__


#include <mutex>
#include <netdb.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unordered_map>

class ASocket
{
public:
    enum SOCKET_ROLE{SRVR,CLINT}; 
    ASocket(__socket_type type_,const char* port_,const char* ip_ = "0.0.0.0", SOCKET_ROLE role = SRVR,int socket_fd = -1);    
    virtual ~ASocket();
    virtual void Send(const char* buffer,int size_to_send, int flags = 0) =0;
    virtual void Recive(void* buffer, int size_to_read, int flags = 0)=0;
    virtual void Response(const char* buffer,int size_to_send, int flags = 0)=0;
    int GetSocketFD() const;

protected:
int m_socket_fd;
sockaddr_storage other_addr;
addrinfo* m_addrinfo;
struct sockaddr_in m_addr;

private:
static std::mutex m_mutex;
static std::unordered_map<int, addrinfo*> m_open_sockets;

};

class UDPSocket:public ASocket
{
public:
    UDPSocket(const char* port_,const char* ip_ = "0.0.0.0", SOCKET_ROLE role = SRVR,int socket_fd = -1);
    virtual ~UDPSocket() = default;
    virtual void Send(const char* buffer,int size_to_send, int flags = 0)override;
    virtual void Recive(void* buffer, int size_to_read, int flags = 0)override;
    virtual void Response(const char* buffer,int size_to_send, int flags = 0)override;

};

class TCPSocket:public ASocket
{
public:
    TCPSocket(const char* port_,const char* ip_ = "0.0.0.0", SOCKET_ROLE role = SRVR,int socket_fd = -1);
    virtual ~TCPSocket() = default;
    virtual void Send(const char* buffer,int size_to_send, int flags = 0)override;
    virtual void Recive(void* buffer, int size_to_read, int flags = 0)override;
    virtual void Response(const char* buffer,int size_to_send, int flags = 0)override;
    void TCPListen();
    void TCPConnect();
    int TCPAccept();
    int GetSocketFD() const;
    int GetChildSocketFD() const;

private:
    int m_child_socket;
    SOCKET_ROLE m_role;
    
};


#endif //__SOCKET_HPP__