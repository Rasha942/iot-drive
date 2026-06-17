#ifndef __ILRD_ARGSKEY__HPP__
#define __ILRD_ARGSKEY__HPP__

#include <memory> //shared_ptr
#include <sys/types.h> // u_int_t

#include "message.hpp"
class IArgsKey
{
public:
    virtual int GetKey() = 0;
    virtual ~IArgsKey() = default;
};

class ReadArgsKey : public IArgsKey
{
public:
    ReadArgsKey(int key_, u_int64_t offset, u_int32_t to_read);
    virtual ~ReadArgsKey() = default;
    virtual int GetKey() override;
    u_int64_t GetOffset();
    u_int32_t GetSize();
    
private:
    int m_key;
    u_int64_t m_offset;
    u_int32_t m_size_to_read;

};

class WriteArgsKey : public IArgsKey
{
public:
    WriteArgsKey(int key_, u_int64_t offset_, u_int32_t to_read_, std::shared_ptr<char[]> data_);
    virtual ~WriteArgsKey();
    virtual int GetKey() override;
    u_int64_t GetOffset();
    u_int32_t GetSize();
    std::shared_ptr<char[]> GetData();
private:
    int m_key;
    u_int64_t m_offset;
    u_int32_t m_size_to_write;
    std::shared_ptr<char[]> m_data;
    
};


class MinionReadArgsKey : public IArgsKey
{
public:
    MinionReadArgsKey(int key_, u_int64_t offset_, u_int32_t size_to_read_,ThreadSafeUID uid_);
    virtual ~MinionReadArgsKey() = default;
    virtual int GetKey() override;
    u_int64_t GetOffset();
    u_int32_t GetSize();
    ThreadSafeUID&& GetUID();

    
private:
    int m_key;
    u_int64_t m_offset;
    u_int32_t m_size_to_read;
    ThreadSafeUID m_uid;
    

};
class MinionWriteArgsKey : public IArgsKey
{
public:
    MinionWriteArgsKey(int key_, u_int64_t offset_, u_int32_t size_to_write_, std::shared_ptr<char[]> data_,ThreadSafeUID uid_);
    virtual ~MinionWriteArgsKey() = default;
    virtual int GetKey() override;
    u_int64_t GetOffset();
    u_int32_t GetSize();
    std::shared_ptr<char[]> GetData();
    ThreadSafeUID&& GetUID();
private:
    int m_key;
    u_int64_t m_offset;
    u_int32_t m_size_to_write;
    std::shared_ptr<char[]> m_data;
    ThreadSafeUID m_uid;
    
};
#endif //__ILRD_ARGSKEY__HPP__