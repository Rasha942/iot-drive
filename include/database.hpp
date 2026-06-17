#ifndef __IRLD_DATA_BASE_HPP__
#define __IRLD_DATA_BASE_HPP__
#include "singleton.hpp"
#include <sqlite3.h> 
class DataBase
{
public:
    ~DataBase();
    void Write();
    friend class Singleton<DataBase>;
private:
    DataBase();
    sqlite3* m_db;
    char* m_messagge_error;

};



#endif //__IRLD_DATA_BASE_HPP__