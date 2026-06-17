
#include <cstring> //memcpy
#include <iostream>
#include <string>
#include <sys/types.h>
#include "message.hpp"
#include "error_handler.hpp"
#include "socket.hpp"

/*******************************************************************************
 *  Class(inheritance):AMessage, ReadMsg(Imessage), WriteMsg(Imessage)                                  *                 
*  ctor Parameters:  non e                                                     *
 *  mathods: Execute(shared_ptr<IArgsKey>)                                     *
 *  life flow: Created at Factory::Create  and sent by refrence to TPTask      *
 *  Purpose: request read/write ops from MinionProxy via   ResponseManeger     *
 *******************************************************************************/

AMessage::AMessage(u_int32_t key_,u_int32_t buffer_size_,ThreadSafeUID uid_):m_key(key_),m_size(buffer_size_),m_uid(uid_){}

ThreadSafeUID AMessage::GetUID()
{
    return std::move(m_uid);
}

u_int32_t AMessage::GetSize() const
{
    return m_size;
}

WriteMsg::WriteMsg(u_int64_t offset_,u_int32_t size_to_write_,std::shared_ptr<char[]>data_,ThreadSafeUID uid_):AMessage(WRITE_MSG_KEY,size_to_write_,uid_),m_offset(offset_),m_data(data_)
{
}
   
ReadMsg::ReadMsg(u_int64_t offset_,u_int32_t size_to_read_, ThreadSafeUID uid_):AMessage(READ_MSG_KEY, size_to_read_,uid_),m_offset(offset_)
{
}

MinionReadMsg::MinionReadMsg(const ThreadSafeUID& uid_, bool is_success_,std::shared_ptr<char[]> data_,u_int32_t size_to_read_):AMessage(READ_MINION_MSG_KEY, size_to_read_ ,uid_),m_is_success(is_success_),m_data(data_)
{
}

MinionWriteMsg::MinionWriteMsg(const ThreadSafeUID& uid_, bool is_success_):AMessage(WRITE_MINION_MSG_KEY, 0 ,uid_),m_is_success(is_success_)
{
}


NBDReadMsg::NBDReadMsg(u_int64_t offset_,u_int32_t len_):AMessage(NBD_READ_MSG, len_),m_offset(offset_)
{
}



NBDWriteMsg::NBDWriteMsg(u_int64_t offset_, u_int32_t len_, const char* data_):AMessage(NBD_WRITE_MSG, len_),m_offset(offset_),m_data(NULL)
{
    if (NULL != data_)
    {
        m_data.reset(new char [len_],std::default_delete<char[]>());
        memcpy(m_data.get(), data_, len_);
    }
}



/****************************************************************
 *  function:ToBuffer           *                   
 *  Purpose: converting data to buffer for udp communication                     *
 *  Parameters(type): none                                      *
 *  Returns: char*                        *
 *  Returns to:                                         *                                       
 ****************************************************************/
char* AMessage::ToBuffer(char* buffer) 
{

    *(u_int32_t*)buffer = m_key;
    //std::cout<< __PRETTY_FUNCTION__ <<" buffer(key) =  " << *(u_int32_t*)buffer << std::endl;
    buffer += sizeof(u_int32_t);
    *(u_int32_t*)buffer = m_size;
    buffer += sizeof(u_int32_t);
    buffer = m_uid.ToBuffer(buffer);
    // buffer += sizeof(ThreadSafeUID);
    return buffer;
}

char* ReadMsg::ToBuffer(char* buffer) 
{
    char* start = buffer;
    buffer = AMessage::ToBuffer(buffer);
    *(u_int64_t*)buffer = m_offset;
    buffer += sizeof(u_int64_t);
    size_t length = buffer - start; // Actual written length
    uint8_t checksum = AMessage::CalculateChecksum(start, length);

        *(uint8_t*)buffer = checksum;
    buffer += sizeof(uint8_t);
    return buffer;
}

char* WriteMsg::ToBuffer(char* buffer) 
{
    char* start = buffer;
    buffer = AMessage::ToBuffer(buffer);
    *(u_int64_t*)buffer = m_offset;
    buffer += sizeof(u_int64_t);
    if (m_data) {
        memcpy(buffer, m_data.get(), GetSize());
    }
    buffer += GetSize();
    size_t length = buffer - start; // Actual written length
    uint8_t checksum = AMessage::CalculateChecksum(start, length);
    
    *(uint8_t*)buffer = checksum;
        
    buffer += sizeof(uint8_t);
    return buffer;
}

char* MinionReadMsg::ToBuffer(char* buffer)
{
    buffer = AMessage::ToBuffer(buffer);
    *(bool*)buffer = m_is_success;
    buffer += sizeof(bool);
    buffer = (char*)memcpy(buffer, m_data.get(), GetSize());
    return buffer; 
}

char* MinionWriteMsg::ToBuffer(char* buffer)
{
    buffer = AMessage::ToBuffer(buffer);
    *(bool*)buffer = m_is_success;
    buffer += sizeof(bool);
    return buffer; 
}


char* NBDWriteMsg::ToBuffer(char* buffer)
{
    buffer = AMessage::ToBuffer(buffer);
    
    *(u_int64_t*)buffer = m_offset;
    std::cout << __PRETTY_FUNCTION__ <<  "  offset(buf) = " << *(u_int64_t*)buffer << std::endl;
    buffer += sizeof(u_int64_t);
    memcpy(buffer,m_data.get(), GetSize());
    buffer += GetSize();
    
    return buffer;
}

char* NBDReadMsg::ToBuffer(char* buffer)
{
    buffer = AMessage::ToBuffer(buffer);
    *(u_int64_t*)buffer= m_offset;
    std::cout << __PRETTY_FUNCTION__ <<  "  offset(buff) = " << *(u_int64_t*)buffer << std::endl;
    buffer += sizeof(u_int64_t);
    
    return buffer;
}

char* ThreadSafeUID::ToBuffer(char* buffer)  
{ 
    *(u_int32_t*)buffer = m_unique_counter;
    buffer += sizeof(u_int32_t);
    *(u_int64_t*)buffer = m_timestamp;
    buffer += sizeof(u_int64_t);
    *(pid_t*)buffer = m_pid;
    buffer +=  sizeof(pid_t);
    *(u_int64_t*)buffer = m_ip;    ;    
    buffer += sizeof(u_int64_t);
    
    return buffer;
}
/****************************************************************
 *  function:FromBuffer           *                   
 *  Purpose: converting data from buffer for initialzing data
    members and  udp communication                     *
 *  Parameters(type): none                                      *
 *  Returns: char*                        *
 *  Returns to:                                         *                                       
 ****************************************************************/


char* AMessage::FromBuffer(char* buffer)
{
    //std::cout<< __PRETTY_FUNCTION__ <<" buffer(key) =  " << *(u_int32_t*)buffer << std::endl;
    m_key =  *(u_int32_t*)buffer;
    buffer += sizeof(u_int32_t);
    //std::cout<< __PRETTY_FUNCTION__ <<" buffer(size) =  " << *(u_int32_t*)buffer << std::endl;
    m_size = *(u_int32_t*)buffer;
    buffer += sizeof(u_int32_t);
    buffer = m_uid.FromBuffer(buffer);
    return buffer;
}

char* ReadMsg::FromBuffer(char* buffer) 
{
    char* start = buffer;  
    buffer = AMessage::FromBuffer(buffer);
    m_offset =  *(u_int64_t*)buffer;
    buffer += sizeof(u_int64_t);
    size_t length = buffer - start; 
    uint8_t calculated_checksum = AMessage::CalculateChecksum(start, length);
    uint8_t received_checksum = *(uint8_t*)buffer;
    buffer += sizeof(uint8_t);
    #ifndef NDEBUG
    CheckAndLog::NotEqualToCheck(received_checksum, calculated_checksum,"Checksum Check",__FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    CheckAndLog::JustLog("Checksum (calculated):" + std::to_string(calculated_checksum) +  ", Checksum (received): " + std::to_string(calculated_checksum),__FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG
    return buffer;
}

char* WriteMsg::FromBuffer(char* buffer) 
{
    char* start = buffer;
    buffer = AMessage::FromBuffer(buffer);
    m_offset = *(u_int64_t*)buffer;
    buffer += sizeof(u_int64_t);
    m_data = std::shared_ptr<char[]>(new char[GetSize()], std::default_delete<char[]>());
    memcpy(m_data.get(), buffer, GetSize());
    buffer += GetSize();
    size_t length = buffer - start;
    uint8_t calculated_checksum = AMessage::CalculateChecksum(start, length);
    uint8_t received_checksum = *(uint8_t*)buffer;
    buffer += sizeof(uint8_t);
    #ifndef NDEBUG
    CheckAndLog::NotEqualToCheck(received_checksum, calculated_checksum,"Checksum Check",__FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    CheckAndLog::JustLog("Checksum (calculated):" +std::to_string(calculated_checksum) +  ", Checksum (received): " + std::to_string(calculated_checksum),__FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG


    return buffer;
}

char* MinionReadMsg::FromBuffer(char* buffer)
{
    buffer = AMessage::FromBuffer(buffer);
    m_is_success = *(bool*)buffer;
    buffer += sizeof(bool);
    m_data = std::shared_ptr<char[]>(new char [GetSize()],std::default_delete<char[]>());
    memcpy(m_data.get(), buffer,GetSize());
    buffer += GetSize();

    return buffer;
}

char* MinionWriteMsg::FromBuffer(char* buffer)
{
    buffer = AMessage::FromBuffer(buffer);
    m_is_success = *(bool*)buffer;
    buffer += sizeof(bool);
    return buffer;
}



char* NBDWriteMsg::FromBuffer(char* buffer)
{
    buffer = AMessage::FromBuffer(buffer);
    m_offset = *(u_int64_t*)buffer;
    std::cout << __PRETTY_FUNCTION__ <<  "  offset(buff) = " << m_offset<< std::endl;
    buffer += sizeof(u_int64_t);
    m_data = std::shared_ptr<char[]>(new char [GetSize()],std::default_delete<char[]>());
    memcpy(m_data.get(), buffer, GetSize());
    buffer += GetSize();
    
    // std::cout << " len offset data"  << GetSize() << m_offset << m_data << std::endl;
    return buffer;
    
}


char* NBDReadMsg::FromBuffer(char* buffer)
{
    buffer = AMessage::FromBuffer(buffer);
    m_offset = *(u_int64_t*)buffer;
    buffer += sizeof(u_int64_t);
    std::cout<< __PRETTY_FUNCTION__ <<  " offset = "  << m_offset << std::endl;
    return buffer;    
}

char* ThreadSafeUID::FromBuffer(char* buffer)  
{ 
    // buffer += sizeof(IMessage);
    m_unique_counter = *(u_int32_t*)buffer;
    buffer += sizeof(u_int32_t);
    m_timestamp = *(u_int64_t*)buffer;
    buffer += sizeof(u_int64_t);
    m_pid = *(pid_t*)buffer;
    buffer +=  sizeof(pid_t);
    m_ip = *(u_int64_t*)buffer;    
    buffer += sizeof(u_int64_t);
    
    return buffer;
}

/****************************************************************
 *  function:GetBufferSize           *                   
 *  Purpose: converting data from buffer for initialzing data
    members and  udp communication                     *
 *  Parameters(type): none                                      *
 *  Returns: char*                        *
 *  Returns to:                                         *                                       
 ****************************************************************/
u_int64_t AMessage::GetBufferSize()
{
    return sizeof(u_int32_t) + (sizeof(u_int32_t))  + m_uid.GetBufferSize();
}
u_int64_t ReadMsg::GetBufferSize()
{
    return AMessage::GetBufferSize() + sizeof(u_int64_t)+sizeof(uint8_t);
}
u_int64_t WriteMsg::GetBufferSize()
{
    return AMessage::GetBufferSize() + sizeof(u_int64_t)+ GetSize() +sizeof(uint8_t);
}

u_int64_t MinionReadMsg::GetBufferSize()
{
    return  AMessage::GetBufferSize() + sizeof(bool);
}

u_int64_t MinionWriteMsg::GetBufferSize()
{
    return  AMessage::GetBufferSize() + sizeof(bool);
}

u_int64_t NBDWriteMsg::GetBufferSize() 
{
    return AMessage::GetBufferSize() + sizeof(u_int64_t) + GetSize();
}

u_int64_t NBDReadMsg::GetBufferSize() 
{
    return AMessage::GetBufferSize() + sizeof(u_int64_t);
}

u_int64_t ThreadSafeUID::GetBufferSize()  
{ 
    return  sizeof(u_int32_t) + 
    sizeof(u_int64_t) +
    sizeof(pid_t) + 
    sizeof(u_int64_t);  
}

std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>> WriteMsg::SupplyArgs()
{
  return std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>>({m_offset,GetSize(),m_data});
}

std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>> ReadMsg::SupplyArgs()
{
  return std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>>({m_offset,GetSize(),NULL});

}

std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>> MinionReadMsg::SupplyArgs()
{
    return std::tuple<u_int32_t,u_int32_t,std::shared_ptr<char[]>>({0,GetSize(),m_data});

}

std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>> MinionWriteMsg::SupplyArgs()
{
    return std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>>({AMessage::GetUID().GetUniqueCounter(),m_is_success,NULL});

}

std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>> NBDReadMsg::SupplyArgs()
{
  return std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>>({m_offset,GetSize(),NULL});
}

std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>> NBDWriteMsg::SupplyArgs()
{
  return std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>>({m_offset,GetSize(),m_data});
}



bool MinionReadMsg::GetIsSuccess()
{
    return m_is_success;
}


bool MinionWriteMsg::GetIsSuccess()
{
    return m_is_success;
}


uint8_t AMessage::CalculateChecksum(const char* data, size_t length) 
{
    uint8_t checksum = 0;
    for (size_t i = 0; i < length; ++i) 
    {
        checksum ^= data[i];
    }
    return checksum;
}



std::atomic_int32_t ThreadSafeUID::m_static_counter = 1;

ThreadSafeUID::ThreadSafeUID():m_unique_counter(atomic_fetch_add(&m_static_counter, 1)),
m_timestamp(time(nullptr)),m_pid(getpid()),m_ip(m_unique_counter + 1)
{
    CheckAndLog::NegativeCheck(m_timestamp, "time()",__FILE_NAME__, __LINE__, __PRETTY_FUNCTION__);
    #ifndef NDEBUG
    CheckAndLog::JustLog("UID created",__FILE_NAME__, __LINE__, __PRETTY_FUNCTION__);
    #endif // NDEBUG
}
ThreadSafeUID::ThreadSafeUID(const ThreadSafeUID& other_):m_unique_counter(other_.m_unique_counter),m_timestamp(other_.m_timestamp),
m_pid(other_.m_pid),m_ip(other_.m_ip)
{
}

ThreadSafeUID::ThreadSafeUID(ThreadSafeUID&& other_):m_unique_counter(std::move(other_.m_unique_counter)),
m_timestamp(std::move(other_.m_timestamp)),m_pid(std::move(other_.m_pid)),m_ip(std::move(other_.m_ip))
{    
}

bool ThreadSafeUID::operator==(const ThreadSafeUID& other_)
{

	if (this->m_unique_counter != other_.m_unique_counter || difftime(this->m_timestamp, other_.m_timestamp) != 0 || this->m_pid != other_.m_pid || this->m_ip != other_.m_ip)
	{
		return false;
	}
    return true;
}

u_int32_t ThreadSafeUID::GetUniqueCounter() const
{
    return m_unique_counter;
}
/******************************
*                             *
*   Creators for Factory      *
*                             *
*******************************/
std::shared_ptr<MinionReadMsg> CreateMinionReadMsg()
{
    return std::make_shared<MinionReadMsg>();
}
std::shared_ptr<MinionWriteMsg> CreateMinionWriteMsg()
{
    return std::make_shared<MinionWriteMsg>();
}

std::shared_ptr<NBDReadMsg> CreateNBDReadMsg()
{
    return std::make_shared<NBDReadMsg>();
}
std::shared_ptr<NBDWriteMsg> CreateNBDWriteMsg()
{
      return std::make_shared<NBDWriteMsg>(); 
}

std::shared_ptr<WriteMsg> CreateWriteMsg()
{
    return std::make_shared<WriteMsg>();
}

std::shared_ptr<ReadMsg> CreateReadMsg()
{
    return std::make_shared<ReadMsg>();
}
