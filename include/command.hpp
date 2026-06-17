#ifndef __IRLD_COMMAND_HPP__
#define __IRLD_COMMAND_HPP__

#include <chrono> //milliseconds
#include <functional> // std::function
#include <memory> // shared_ptr
#include <utility> // std::pair
#include <fstream>

#include "argskey.hpp"

extern std::fstream bin_file;


class ICommand
{
public:
    virtual std::pair<std::function<bool()>,std::chrono::milliseconds> Execute(std::shared_ptr<IArgsKey>task_args) = 0;
};

class ReadCommand : public ICommand
{
public:
    virtual std::pair<std::function<bool()>,std::chrono::milliseconds> Execute(std::shared_ptr<IArgsKey>task_args);
};

class WriteCommand : public ICommand
{
public:
    virtual std::pair<std::function<bool()>,std::chrono::milliseconds> Execute(std::shared_ptr<IArgsKey>task_args);
};


class AMinionCmd : public ICommand
{
public:
    std::fstream& GetBinFile();     
};
class MinionReadCmd : public AMinionCmd
{
    virtual std::pair<std::function<bool()>,std::chrono::milliseconds> Execute(std::shared_ptr<IArgsKey>task_args);
};

class MinionWriteCmd : public AMinionCmd
{
    virtual std::pair<std::function<bool()>,std::chrono::milliseconds> Execute(std::shared_ptr<IArgsKey>task_args);
};






std::shared_ptr<ICommand> CreateWriteCommand();

std::shared_ptr<ICommand> CreateReadCommand();

std::shared_ptr<ICommand> CreateMinionWriteCmd();

std::shared_ptr<ICommand> CreateMinionReadCmd();



struct ReadAIFunc
{
    ReadAIFunc(const ThreadSafeUID& uid_); 
    bool operator()();
    
private:
    std::shared_ptr<ThreadSafeUID> m_uid;
};  


struct WriteAIFunc
{
    WriteAIFunc(const ThreadSafeUID& og_uid_, const ThreadSafeUID& backup_uid_); 
    bool operator()();
    
private:
    std::shared_ptr<ThreadSafeUID> m_og_uid;
    std::shared_ptr<ThreadSafeUID> m_backup_uid;   
};  
#endif // __IRLD_COMMAND_HPP__
