
#include "argskey.hpp"


ReadArgsKey::ReadArgsKey(int key_, u_int64_t offset_, u_int32_t size_to_read_):m_key(key_),m_offset(offset_),m_size_to_read(size_to_read_){}

int ReadArgsKey::GetKey()
{
    return m_key;
}

u_int64_t ReadArgsKey::GetOffset()
{
    return m_offset;
}

u_int32_t ReadArgsKey::GetSize()
{
    return m_size_to_read;
}

WriteArgsKey::WriteArgsKey(int key_, u_int64_t offset_, u_int32_t size_to_write_, std::shared_ptr<char[]> data_): m_key(key_), m_offset(offset_),m_size_to_write(size_to_write_),m_data(data_)
{
}
WriteArgsKey::~WriteArgsKey()
{
}

int WriteArgsKey::GetKey()
{
    return m_key;
}

u_int64_t WriteArgsKey::GetOffset()
{
    return m_offset;
}

u_int32_t WriteArgsKey::GetSize()
{
    return m_size_to_write;
}


std::shared_ptr<char[]> WriteArgsKey::GetData()
{
    return m_data;
}

MinionWriteArgsKey::MinionWriteArgsKey(int key_, u_int64_t offset_, u_int32_t size_to_write_, std::shared_ptr<char[]> data_,ThreadSafeUID uid_):
m_key(key_),m_offset(offset_),m_size_to_write(size_to_write_),m_data(data_),m_uid(std::move(uid_))
{    
}
    // virtual ~MinionWriteArgsKey();

int MinionWriteArgsKey::GetKey()
{
    return m_key;
}
u_int64_t MinionWriteArgsKey::GetOffset()
{
    return m_offset;
}

u_int32_t MinionWriteArgsKey::GetSize()
{
    return m_size_to_write;
}

std::shared_ptr<char[]> MinionWriteArgsKey::GetData()
{
    return m_data;
}

ThreadSafeUID&& MinionWriteArgsKey::GetUID() 
{
    return std::move(m_uid);
}



MinionReadArgsKey::MinionReadArgsKey(int key_, u_int64_t offset_, u_int32_t size_to_read_, ThreadSafeUID uid_):
m_key(key_),m_offset(offset_),m_size_to_read(size_to_read_),m_uid(std::move(uid_))
{   
}

int MinionReadArgsKey::GetKey()
{
    return m_key;
}
u_int64_t MinionReadArgsKey::GetOffset()
{
    return m_offset;
}

u_int32_t MinionReadArgsKey::GetSize()
{
    return m_size_to_read;
}

ThreadSafeUID&& MinionReadArgsKey::GetUID() 
{
    return std::move(m_uid);
}
