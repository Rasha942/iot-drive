#ifndef __IRLD_MESSAGE_HPP__
#define __IRLD_MESSAGE_HPP__

#include <atomic> //std::atomic
#include <memory> // shared_ptr
#include <sys/types.h> // u_int_t
#include <tuple> // tuple


#define WRITE_MSG_KEY 8
#define READ_MSG_KEY 9
#define READ_MINION_MSG_KEY 28
#define WRITE_MINION_MSG_KEY 29
#define NBD_WRITE_MSG 30
#define NBD_READ_MSG 31

class IMessage
{
public:
    virtual char* ToBuffer(char* buffer)  = 0;
    virtual char* FromBuffer(char* buffer) = 0;
    virtual u_int64_t GetBufferSize() = 0;

};

class ThreadSafeUID : public IMessage
{
public:
    explicit ThreadSafeUID();
    ThreadSafeUID(ThreadSafeUID&& other_);
    ThreadSafeUID(const ThreadSafeUID& other_);
    bool operator==(const ThreadSafeUID& other_);
    u_int32_t GetUniqueCounter() const;
    virtual char* ToBuffer(char* buffer) override;
    virtual char* FromBuffer(char* buffer) override;
    virtual u_int64_t GetBufferSize() override;
private:

    u_int32_t m_unique_counter;
    u_int64_t m_timestamp;
    pid_t m_pid;
    u_int64_t m_ip;
    static std::atomic_int32_t m_static_counter;
};


class AMessage : public IMessage
{
public:
    AMessage(u_int32_t key_, u_int32_t buffer_size_, ThreadSafeUID uid_ = ThreadSafeUID());
    virtual char* ToBuffer(char* buffer) override;
    virtual char* FromBuffer(char* buffer)override;
    virtual  u_int64_t GetBufferSize()override;
    virtual std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>> SupplyArgs() =0;
    ThreadSafeUID GetUID();
    u_int32_t GetSize() const;
    uint8_t CalculateChecksum(const char* data, size_t length);

private:
    u_int32_t m_key;
    u_int32_t m_size;
   ThreadSafeUID m_uid;
};

class ReadMsg :public AMessage
{
public:
    ReadMsg(u_int64_t offset_ = 0,u_int32_t size_to_read_ = 0,ThreadSafeUID uid_ = ThreadSafeUID());
    virtual char* ToBuffer(char* data)override;
    virtual char* FromBuffer(char* data)override;
    virtual u_int64_t GetBufferSize()override;
    virtual std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>> SupplyArgs() override;
private:
    u_int64_t m_offset;
};


class NBDReadMsg : public AMessage
{
public:
    NBDReadMsg(u_int64_t offset_ = 0, u_int32_t len_ = 0);
    virtual char* ToBuffer(char* buffer)override;
    virtual char* FromBuffer(char* buffer)override;
    virtual u_int64_t GetBufferSize() override;
    std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>> SupplyArgs()override;

private:
    u_int64_t m_offset;
};

class NBDWriteMsg : public AMessage
{
public:
    NBDWriteMsg(u_int64_t offset_ = 0, u_int32_t len_ = 0, const char* data_ = NULL);
    virtual char* ToBuffer(char* buffer)override;
    virtual char* FromBuffer(char* buffer)override;
    virtual u_int64_t GetBufferSize() override;
    std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>> SupplyArgs()override;

private:
    u_int64_t m_offset;
    std::shared_ptr<char[]> m_data;
};




class WriteMsg :public AMessage
{
public:
    WriteMsg(u_int64_t offset_ = 0,u_int32_t size_to_write = 0,std::shared_ptr<char[]>data_ = NULL, ThreadSafeUID uid_ = ThreadSafeUID());
    virtual char* ToBuffer(char* data)override;
    virtual char* FromBuffer(char* data)override;
    virtual u_int64_t GetBufferSize()override;
    virtual std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>>SupplyArgs() override;

private:
    u_int64_t m_offset;
    std::shared_ptr<char[]> m_data;
};

class MinionReadMsg : public AMessage
{
public:
    MinionReadMsg(const ThreadSafeUID& uid_ = ThreadSafeUID(), bool is_success_ = true, std::shared_ptr<char[]> data = NULL, u_int32_t size_to_read_ = 0);
    virtual char* ToBuffer(char* data)override;
    virtual char* FromBuffer(char* data)override;
    virtual u_int64_t GetBufferSize()override;
    bool GetIsSuccess();
    virtual std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>>SupplyArgs() override;
private:
    bool m_is_success;
    std::shared_ptr<char[]> m_data;

};

class MinionWriteMsg : public AMessage
{
public:
    MinionWriteMsg(const ThreadSafeUID& uid_ = ThreadSafeUID(), bool is_success_ = true);
    virtual char* ToBuffer(char* data)override;
    virtual char* FromBuffer(char* data)override;
    virtual u_int64_t GetBufferSize()override;
    bool GetIsSuccess();
    virtual std::tuple<u_int64_t,u_int32_t,std::shared_ptr<char[]>>SupplyArgs() override;
private:
    bool m_is_success;
};
std::shared_ptr<WriteMsg> CreateWriteMsg();


std::shared_ptr<ReadMsg> CreateReadMsg();

std::shared_ptr<MinionReadMsg> CreateMinionReadMsg();

std::shared_ptr<MinionWriteMsg> CreateMinionWriteMsg();

std::shared_ptr<NBDReadMsg> CreateNBDReadMsg();

std::shared_ptr<NBDWriteMsg> CreateNBDWriteMsg();
#endif //__IRLD MESSAGE_HPP__