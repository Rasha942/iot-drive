#include <cstring>
#include <sys/types.h>
#include <thread>
#include "inputproxy.hpp"
#include "error_handler.hpp"
#include "factory.hpp"
#include "logger.hpp"
#include "message.hpp"
#include "response_manger.hpp"
#include "raid01.hpp"
#include "singleton.hpp"
#include "socket.hpp"
#include "command.hpp"

std::fstream bin_file;

/****************************************************************
 *  Class: NBDProxy                                             *
 *  ctor Parameters: ctor (none)                                *
 *  mathods: GetArgsKey, NotifySucces                           *
 *  life flow: sent from main to: the framework ctor->          *
 *  InputMediator  ctor-> InputMediator::ReactorCallback        *
 *  Purpose:  supplies WriteArgsKey & ReadArgsKey               *                                    
 ****************************************************************/

NBDProxy::NBDProxy():m_socket(std::make_shared<TCPSocket>(TCP_PORT))
{
    Singleton<Factory<int, std::shared_ptr<NBDReadMsg>>>::GetInstance()->AddCtor(NBD_READ_MSG, CreateNBDReadMsg);
    Singleton<Factory<int, std::shared_ptr<NBDWriteMsg>>>::GetInstance()->AddCtor(NBD_WRITE_MSG, CreateNBDWriteMsg);
    m_socket->TCPListen();
           std::thread([]() {
        NBD::Run();
    }).detach();

    m_socket->TCPAccept();

          std::thread([]() {
        NBD::MKFSAndMount();
    }).detach();
   
}



int NBDProxy::GetSocketFD()
{
    return m_socket->GetChildSocketFD();
}

/****************************************************************
 *  function: NBDPROXY :: GetArgsKey                            *                   
 *  Purpose:  supplies WriteArgsKey & ReadArgsKey               *                                    
 *  Parameters(type): fd (int), mode(Reactor::Mode)             *
 *  Returns: std::shared_ptr<IArgsKey>                          *
 *  Ret urns to: InputMediator::ReactorCallback::operator()      *                                       
 ****************************************************************/

std::shared_ptr<IArgsKey> NBDProxy::GetArgsKey(int fd, Reactor::Mode mode)
{
    #ifndef NDEBUG
    CheckAndLog::JustLog("IN", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG
    (void)mode;

    char* key_and_size = new char [sizeof(int) + sizeof(u_int32_t)];
    m_socket->Recive(key_and_size, sizeof(int) + sizeof(u_int32_t),MSG_PEEK);
    int key = *(int*)key_and_size;
    key_and_size += sizeof(int);
    u_int32_t size = *(u_int32_t*)key_and_size;
    if(NBD_WRITE_MSG == key)
    {
        std::shared_ptr<NBDWriteMsg> nbd_msg = Singleton<Factory<int, std::shared_ptr<NBDWriteMsg>>>::GetInstance()->Create(key);
        std::shared_ptr<char[]> buffer(new char[size + nbd_msg->GetBufferSize()],std::default_delete<char[]>()); 
        m_socket->Recive(buffer.get(), nbd_msg->GetBufferSize() + size);
        nbd_msg->FromBuffer(buffer.get());
        std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>> args = nbd_msg->SupplyArgs();
       
        #ifndef NDEBUG
        CheckAndLog::JustLog("OUT", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
        #endif //NDEBUG

        return std::make_shared<WriteArgsKey>(WRITE_CMD_KEY,std::get<0>(args),std::get<1>(args),std::get<2>(args));
    }
    std::shared_ptr<NBDReadMsg> nbd_msg = Singleton<Factory<int, std::shared_ptr<NBDReadMsg>>>::GetInstance()->Create(key);
    char* buffer = new char[nbd_msg->GetBufferSize()]; 
    m_socket->Recive(buffer, nbd_msg->GetBufferSize());
    nbd_msg->FromBuffer(buffer);
    std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>> args = nbd_msg->SupplyArgs();
    
    #ifndef NDEBUG
    CheckAndLog::JustLog("OUT", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG

    return std::make_shared<ReadArgsKey>(READ_CMD_KEY,std::get<0>(args),std::get<1>(args));

}

void NBDProxy::NotifySucces(const char* data, u_int32_t size)
{   
    m_socket->Response(data,size);    
}



/****************************************************************
 *  Class: IMinion & MinionProxy                                *
 *  ctor Parameters: none                                       *
 *  mathods: ReadRequest & WriteRequest                     *
 *  life flow: Created at Raid01::Raid01 ->                     *
 *   Read/WriteCommand::Execute                                  *
 *  Purpose:  Sends UDP message to a minion  
 *                   *                                    
 ******************************************************************/
IMinion:: ~IMinion()
{

}

MinionProxy:: MinionProxy(const char* port_, const char* ip_):m_port(port_),m_ip(ip_),m_socket(new UDPSocket(port_,ip_,ASocket::CLINT))
{ 
}
 /***************************************************************
 *  function: MinionProxy::ReadRequest                       *                   
 *  Purpose: Send Read Request to a minion via UDP            *
 *  Parameters(type): none                                      *
 *  Returns
 *  Returns to:                                                 *                                       
 ****************************************************************/
void MinionProxy::ReadRequest(u_int64_t offset_, u_int32_t size,ThreadSafeUID uid_)
{
    ReadMsg read_msg(offset_,size,uid_);
    std::shared_ptr<char[]> buffer(new char[read_msg.GetBufferSize()],std::default_delete<char[]>());
    read_msg.ToBuffer(buffer.get());
    m_socket->Send(buffer.get(),read_msg.GetBufferSize());  
}
/****************************************************************
 *  function: MinionProxy::WriteRequest                        *                   
 *  Purpose: Sends Write Request to a minion via UDP            *
 *  Parameters(type): none                                      *
 *  Returns:                                                    *
 *  Returns to:                                                 *                                       
 *************************************************************>>***/
void MinionProxy::WriteRequest(u_int64_t offset_,u_int32_t size, std::shared_ptr<char[]> data, ThreadSafeUID uid_)
{
    #ifndef NDEBUG
    CheckAndLog::JustLog("IN", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG
    WriteMsg write_msg(offset_,size,data,uid_);
    std::shared_ptr<char[]> buffer(new char[write_msg.GetBufferSize()],std::default_delete<char[]>());
    write_msg.ToBuffer(buffer.get());
    m_socket->Send(buffer.get(),write_msg.GetBufferSize());
    #ifndef NDEBUG
    CheckAndLog::JustLog("OUT", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG
}



ResponseProxy::ResponseProxy():m_socket(std::make_shared<UDPSocket>(UDP_PORT_MASTER,"127.0.1.1"))
{
    Singleton<Factory<int, std::shared_ptr<MinionReadMsg>>>::GetInstance()->AddCtor(READ_MINION_MSG_KEY, CreateMinionReadMsg);
    Singleton<Factory<int, std::shared_ptr<MinionWriteMsg>>>::GetInstance()->AddCtor(WRITE_MINION_MSG_KEY, CreateMinionWriteMsg);
}


int ResponseProxy::GetSocketFD()
{
    return m_socket->GetSocketFD();
}
std::shared_ptr<IArgsKey> ResponseProxy::GetArgsKey(int fd, Reactor::Mode)
{
    #ifndef NDEBUG
    CheckAndLog::JustLog("IN", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG
    char * key_and_size = new char [sizeof(u_int32_t) + sizeof(u_int32_t)];
    m_socket->Recive(key_and_size,sizeof(u_int32_t) + sizeof(u_int32_t),MSG_PEEK);
    u_int32_t key = *(u_int32_t*)key_and_size;
    key_and_size += sizeof(u_int32_t);
    u_int32_t re_data_size = *(u_int32_t*)key_and_size;

    if(WRITE_MINION_MSG_KEY == key)
    {
        std::shared_ptr<MinionWriteMsg> msg = Singleton<Factory<int, std::shared_ptr<MinionWriteMsg>>>::GetInstance()->Create(key);
        char* buffer= new char[msg->GetBufferSize()];
        m_socket->Recive(buffer, msg->GetBufferSize());
        msg->FromBuffer(buffer);
        std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>> args  = msg->SupplyArgs();
        if(true == std::get<1>(args))
        {
            Singleton<ResponseManeger>::GetInstance()->RegisterResponse(msg->GetUID());
        }
        delete [] buffer;
    }
    if(READ_MINION_MSG_KEY == key)
    {
        std::shared_ptr<MinionReadMsg> msg = Singleton<Factory<int, std::shared_ptr<MinionReadMsg>>>::GetInstance()->Create(key);
        char* buffer= new char[msg->GetBufferSize() + re_data_size];
        m_socket->Recive(buffer, msg->GetBufferSize() +  re_data_size);
        msg->FromBuffer(buffer);
        std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>> args  = msg->SupplyArgs();
        if(true == msg->GetIsSuccess())
        {    
            Singleton<ResponseManeger>::GetInstance()->RegisterResponse(msg->GetUID());
            Singleton<NBDProxy>::GetInstance()->NotifySucces(std::get<2>(args).get(),std::get<1>(args));
        }
        delete [] buffer;


    }
    #ifndef NDEBUG
    CheckAndLog::JustLog("OUT", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG
    return NULL;
}
  

/****************************************************************
 *  Class: MasterProxy(IInputProxy)                                             *
 *  ctor Parameters: ctor (none)                                *
 *  mathods:                        *
 *  life flow:         *
 *  Purpose:                *                                    
 ****************************************************************/


MasterProxy::MasterProxy(const char* sock_port):m_socket(std::make_shared<UDPSocket>(sock_port,"127.0.1.1")), m_response_socket(std::make_shared<UDPSocket>(UDP_PORT_MASTER,"127.0.1.1",ASocket::CLINT))
{
    if(strcmp(sock_port, UDP_PORT_MINION_1))
    {
        bin_file.open("minion1.bin", std::ios::binary | std::ios::in | std::ios::out);

    }
    else if(strcmp(sock_port, UDP_PORT_MINION_2))
    {
        bin_file.open("minion2.bin", std::ios::binary | std::ios::in | std::ios::out);

    }
    else
    {
        bin_file.open("minion3.bin", std::ios::binary | std::ios::in | std::ios::out);

    }
    Singleton<Factory<int, std::shared_ptr<WriteMsg>>>::GetInstance()->AddCtor(WRITE_MSG_KEY, CreateWriteMsg);
    Singleton<Factory<int, std::shared_ptr<ReadMsg>>>::GetInstance()->AddCtor(READ_MSG_KEY, CreateReadMsg);
}
int MasterProxy::GetSocketFD() 
{
    return m_socket->GetSocketFD();
} 


std::shared_ptr<IArgsKey> MasterProxy::GetArgsKey(int fd, Reactor::Mode)
{
    #ifndef NDEBUG
    CheckAndLog::JustLog("IN", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG
    char* key_and_size = new char[sizeof(u_int32_t) + sizeof(u_int32_t)];
    m_socket->Recive(key_and_size,sizeof(u_int32_t) + sizeof(u_int32_t),MSG_PEEK);
    int key = *(u_int32_t*)key_and_size;
    //std::cout << __PRETTY_FUNCTION__ << "  key = " << key << std::endl;
    key_and_size += sizeof(u_int32_t);
    u_int32_t size = *(u_int32_t*)key_and_size;
    //std::cout << __PRETTY_FUNCTION__ << "  size = " << size << std::endl;

    if(key == WRITE_MSG_KEY)
    {
        std::shared_ptr<WriteMsg> msg = Singleton<Factory<int, std::shared_ptr<WriteMsg>>>::GetInstance()->Create(key);
        std::shared_ptr<char[]> buffer(new char[msg->GetBufferSize() + size],std::default_delete<char[]>());
        m_socket->Recive(buffer.get(),msg->GetBufferSize() + size );
        msg->FromBuffer(buffer.get());
        std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>>  write_tuple = msg->SupplyArgs();

        #ifndef NDEBUG
        CheckAndLog::JustLog("OUT", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
        #endif //NDEBUG
        
        return std::make_shared<MinionWriteArgsKey>(MINION_WRITE_CMD_KEY,std::get<0>(write_tuple),std::get<1>(write_tuple),std::get<2>(write_tuple),msg->GetUID());
    };
    std::shared_ptr<ReadMsg> msg = Singleton<Factory<int, std::shared_ptr<ReadMsg>>>::GetInstance()->Create(key);
    std::shared_ptr<char[]> buffer(new char[msg->GetBufferSize()],std::default_delete<char[]>());
    m_socket->Recive(buffer.get(),msg->GetBufferSize());
    msg->FromBuffer(buffer.get());
    std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>> read_tuple = msg->SupplyArgs();
    #ifndef NDEBUG
    CheckAndLog::JustLog("OUT", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG
    return std::make_shared<MinionReadArgsKey>(MINION_READ_CMD_KEY,std::get<0>(read_tuple),std::get<1>(read_tuple),msg->GetUID());
}

void MasterProxy::ReadResponse(const ThreadSafeUID& uid_,bool is_success_,std::shared_ptr<char[]> data_, u_int32_t size)
{
    #ifndef NDEBUG
    CheckAndLog::JustLog("IN", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG
    MinionReadMsg msg(uid_,is_success_,data_,size);
    std::shared_ptr<char[]> buffer(new char[msg.GetBufferSize() + size],std::default_delete<char[]>());
    msg.ToBuffer(buffer.get());

    m_response_socket->Send(buffer.get(),msg.GetBufferSize() + size);
    #ifndef NDEBUG
    CheckAndLog::JustLog("OUT", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG
}
void MasterProxy::WriteResponse(const ThreadSafeUID& uid_,bool is_success_)
{
    #ifndef NDEBUG
    CheckAndLog::JustLog("IN", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG

    MinionWriteMsg msg(uid_,is_success_);
    std::shared_ptr<char[]> buffer(new char[msg.GetBufferSize()],std::default_delete<char[]>());
    msg.ToBuffer(buffer.get());
    m_response_socket->Send(buffer.get(),msg.GetBufferSize());
    
    #ifndef NDEBUG
    CheckAndLog::JustLog("OUT", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG
}

