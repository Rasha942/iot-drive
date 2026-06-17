#ifndef __IRLD_INPUT_PROXY__HPP__
#define __IRLD_INPUT_PROXY__HPP__

#include <memory> //shared_ptr

#include "reactor.hpp"
#include "argskey.hpp"
#include "socket.hpp"
#include "message.hpp"
#include "raz_nbd.hpp"
#define READ_CMD_KEY 1
#define WRITE_CMD_KEY 2
#define MINION_READ_CMD_KEY 20
#define MINION_WRITE_CMD_KEY 21
#define TCP_PORT "8080"
#define UDP_PORT_MASTER "6060"

class IInputProxy
{
 public:
    virtual std::shared_ptr<IArgsKey> GetArgsKey(int fd, Reactor::Mode) = 0; // We dont actually need fd & Reactor::Mode, but its convient to have them.
    virtual ~IInputProxy() = default;
    virtual int GetSocketFD() =0;
}; 




class NBDProxy : public IInputProxy
{
public:
    ~NBDProxy() = default;
    virtual std::shared_ptr<IArgsKey> GetArgsKey(int fd, Reactor::Mode)override;
    int GetSocketFD()override;
    void NotifySucces(const char* data = "Success", u_int32_t size = 8);
    friend Singleton<NBDProxy>;
private:
    NBDProxy();
    std::shared_ptr<TCPSocket> m_socket;

};

class ResponseProxy : public IInputProxy
{
public:
    ~ResponseProxy() = default;
    virtual std::shared_ptr<IArgsKey> GetArgsKey(int fd, Reactor::Mode)override;
    virtual int GetSocketFD() override;
    friend class Singleton<ResponseProxy>;

private:
    ResponseProxy();
    std::shared_ptr<ASocket> m_socket;
    std::shared_ptr<char[]> m_data;   
};


class MasterProxy: public IInputProxy
{
public:
    virtual ~MasterProxy() = default;
    virtual std::shared_ptr<IArgsKey> GetArgsKey(int fd, Reactor::Mode) override;
    int GetSocketFD()override;
    void ReadResponse(const ThreadSafeUID& uid_,bool is_success_,std::shared_ptr<char[]> data, u_int32_t size);  
    void WriteResponse(const ThreadSafeUID& uid_,bool is_success_); 
    friend class Singleton<MasterProxy,const char*>;
private:

MasterProxy(const char* sock_port);
std::shared_ptr<ASocket> m_socket;
std::shared_ptr<ASocket> m_response_socket;

};






class IMinion
{
public:
    virtual ~IMinion()=0;
    virtual void ReadRequest(u_int64_t offset_, u_int32_t size,ThreadSafeUID uid_)=0;
    virtual void WriteRequest(u_int64_t offset_,u_int32_t size, std::shared_ptr<char[]> data, ThreadSafeUID uid_) =0;
};
class MinionProxy:public IMinion
{
public:
    MinionProxy(const char* port_, const char* ip_);
    virtual ~MinionProxy() = default;
    void ReadRequest(u_int64_t offset_, u_int32_t size,ThreadSafeUID uid_) override;
    void WriteRequest(u_int64_t offset_,u_int32_t size, std::shared_ptr<char[]> data,ThreadSafeUID uid_) override;
private:
    const char* m_port;
    const char* m_ip;
    std::shared_ptr<ASocket> m_socket;

};
#endif //__IRLD_INPUT_PROXY__HPP__