#include <fstream>
#include <iostream>
#include <sys/types.h>
#include "command.hpp"
#include "inputproxy.hpp"
#include "response_manger.hpp"
#include "raid01.hpp"
#include "argskey.hpp"
#include "error_handler.hpp"




std::pair<std::function<bool()>,std::chrono::milliseconds> ReadCommand::Execute(std::shared_ptr<IArgsKey>task_args)
{
    #ifndef NDEBUG
    CheckAndLog::JustLog("IN", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG

    std::shared_ptr<ReadArgsKey> read_task = std::dynamic_pointer_cast<ReadArgsKey>(task_args);
    
    #ifndef NDEBUG
    CheckAndLog::EqualToCheck<IArgsKey*>(read_task.get(),NULL,"ReadArgsKey NULL Check",__FILE_NAME__,__LINE__, __PRETTY_FUNCTION__);
    #endif //NDEBUG
    
    std::vector<std::pair<std::shared_ptr<MinionProxy>, u_int64_t>> raid_vector = Singleton<Raid01>::GetInstance()->OffsetToProxy(read_task->GetOffset());
    ThreadSafeUID og_uid = Singleton<ResponseManeger>::GetInstance()->RegisterCommand();

    raid_vector[0].first->ReadRequest(raid_vector[0].second,read_task->GetSize(),og_uid);
    ReadAIFunc ai_func(og_uid);
    std::pair<std::function<bool()>,std::chrono::milliseconds> ret_pair(std::make_pair(ai_func, 5000));
    
    #ifndef NDEBUG
    CheckAndLog::JustLog("OUT", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG
    
    return ret_pair;

}

std::pair<std::function<bool()>,std::chrono::milliseconds> WriteCommand::Execute(std::shared_ptr<IArgsKey>task_args)
{
    #ifndef NDEBUG
    CheckAndLog::JustLog("IN", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG

    std::shared_ptr<WriteArgsKey> write_task = std::dynamic_pointer_cast<WriteArgsKey>(task_args);

    #ifndef NDEBUG
    CheckAndLog::EqualToCheck<IArgsKey*>(write_task.get(),NULL,"ArgsKey NULL Check",__FILE_NAME__,__LINE__, __PRETTY_FUNCTION__);
    #endif //NDEBUG

    std::vector<std::pair<std::shared_ptr<MinionProxy>, u_int64_t>>raid10_vector = Singleton<Raid01>::GetInstance()->OffsetToProxy(write_task->GetOffset());
    
    ThreadSafeUID og_uid = Singleton<ResponseManeger>::GetInstance()->RegisterCommand();
    ThreadSafeUID backup_uid = Singleton<ResponseManeger>::GetInstance()->RegisterCommand();
    
    raid10_vector[0].first->WriteRequest(raid10_vector[0].second,write_task->GetSize(), write_task->GetData(),og_uid);
    raid10_vector[1].first->WriteRequest(raid10_vector[1].second,write_task->GetSize(), write_task->GetData(),backup_uid);

    WriteAIFunc ai_func(og_uid,backup_uid);
    std::pair<std::function<bool()>,std::chrono::milliseconds> ret_pair(std::make_pair(ai_func, 500));

    #ifndef NDEBUG
    CheckAndLog::JustLog("OUT", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG

    return ret_pair;

}


std::pair<std::function<bool()>,std::chrono::milliseconds> MinionReadCmd::Execute(std::shared_ptr<IArgsKey>task_args)
{
    #ifndef NDEBUG
    CheckAndLog::JustLog("IN", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG

    std::shared_ptr<MinionReadArgsKey> read_args = std::dynamic_pointer_cast<MinionReadArgsKey>(task_args);
    std::shared_ptr<char[]> data(new char [read_args->GetSize()],std::default_delete<char[]>());
    std::fstream& file = GetBinFile();
    CheckAndLog::EqualToCheck<std::fstream*>(&file, NULL, "fstream NULL check",__FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
            std::cout << "reading from offset : " << read_args->GetOffset() << std::endl;

    file.seekg(read_args->GetOffset(),std::ios::beg);
    file.read(data.get(), read_args->GetSize());
            file.seekg(0);

    MasterProxy* proxy = Singleton<MasterProxy,const char*>::GetInstance("");
    proxy->ReadResponse(read_args->GetUID(), true, data,read_args->GetSize());

    #ifndef NDEBUG
    CheckAndLog::JustLog("OUT", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG

    return std::pair<std::function<bool()>,std::chrono::milliseconds>(nullptr,500);
}

std::pair<std::function<bool()>,std::chrono::milliseconds> MinionWriteCmd::Execute(std::shared_ptr<IArgsKey>task_args)
{
    #ifndef NDEBUG
    CheckAndLog::JustLog("IN", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG
    
    std::shared_ptr<MinionWriteArgsKey> write_args = std::dynamic_pointer_cast<MinionWriteArgsKey>(task_args);

    std::fstream&file = GetBinFile();
    CheckAndLog::EqualToCheck<std::fstream*>(&file, NULL, "fstream NULL check",__FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
        std::cout << "writing to offset : " << write_args->GetOffset() << std::endl;
        file.seekp(write_args->GetOffset(),std::ios::beg);
        file.write(write_args->GetData().get(),write_args->GetSize());
        file.seekp(0);
    
    MasterProxy* proxy = Singleton<MasterProxy,const char*>::GetInstance(NULL);
    std::shared_ptr<char[]> stab(new char[0]);
    proxy->WriteResponse(write_args->GetUID(), true);

    #ifndef NDEBUG
    CheckAndLog::JustLog("OUT", __FILE_NAME__,__LINE__,__PRETTY_FUNCTION__);
    #endif //NDEBUG

    return std::pair<std::function<bool()>,std::chrono::milliseconds>(nullptr,500);
}

std::fstream& AMinionCmd::GetBinFile()
{
    static bool is_init = true;
   
    if(is_init)
    {
        for(int i = 0; i < 8 * MB; ++i)
        {
            char c = 0;
            bin_file.write(&c, sizeof(char));
        }
        is_init = false;
        bin_file.seekp(0);
    }
    return bin_file;
}   


std::shared_ptr<ICommand> CreateWriteCommand()
{
    return std::make_shared<WriteCommand>();
}

std::shared_ptr<ICommand> CreateReadCommand()
{
    return std::make_shared<ReadCommand>();
}

std::shared_ptr<ICommand> CreateMinionWriteCmd()
{
    return std::make_shared<MinionWriteCmd>();
}

std::shared_ptr<ICommand> CreateMinionReadCmd()
{
    return std::make_shared<MinionReadCmd>();
}

ReadAIFunc::ReadAIFunc(const ThreadSafeUID& uid_):m_uid(std::make_shared<ThreadSafeUID>(uid_))
{
}

bool ReadAIFunc::operator()()
{   
    bool ret = Singleton<ResponseManeger>::GetInstance()->HasResponseRecived(*m_uid);
    //std::cout << __func__ << " AI ret =  "<< ret << std::endl;
    if(ret)
    {
        // Singleton<NBDProxy>::GetInstance()->NotifySucces();
    }

    return ret;
}

WriteAIFunc::WriteAIFunc(const ThreadSafeUID& og_uid_, const ThreadSafeUID& backup_uid_):m_og_uid(std::make_shared<ThreadSafeUID>(og_uid_)),
m_backup_uid(std::make_shared<ThreadSafeUID>(backup_uid_))
{
}

bool WriteAIFunc::operator()()
{
    static int og_counter = 0;
    static int backup_counter = 0;
    bool og = Singleton<ResponseManeger>::GetInstance()->HasResponseRecived(*m_og_uid);
    bool backup = Singleton<ResponseManeger>::GetInstance()->HasResponseRecived(*m_backup_uid);
    if(og && backup)
    {
        //std::cout << __func__ << " ret = true" << std::endl;
        Singleton<NBDProxy>::GetInstance()->NotifySucces();
        return true;
    }
    //std::cout << __func__ << " ret = false" << std::endl;
    if(false == og)
    {
        ++og_counter;
    }
    if(false == backup)
    {
        ++backup_counter;
    }
    //std::cout << "og counter = " << og_counter << std::endl;
    //std::cout << "backup counter = " << backup_counter << std::endl;
    if(50 == og_counter || 50 == backup_counter)
    {
        NBDProxy* nbdproxy = Singleton<NBDProxy>::GetInstance();
        og_counter == 50 ? nbdproxy->NotifySucces("couldnt write og", 15) : nbdproxy->NotifySucces("couldnt write backup", 20);
        og_counter = 0;
        backup_counter = 0;
        return true;
    }

        return false;
}

