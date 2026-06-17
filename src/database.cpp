#include "database.hpp"
#include "error_handler.hpp"
#include <string>


DataBase::DataBase()
{
    std::string sql = "CREATE TABLE Request("
    "UID INT PRIMARY KEY     NOT NULL, "
    "TYPE           TEXT    NOT NULL, "
    "MINION_NUM     INT     NOT NULL, "
    "OFFSET         SIZE_T     NOT NULL, "
    "SIZE           INT         NOT NULL);";

    CheckAndLog::NotEqualToCheck(sqlite3_open("database.db", &m_db), 0, "sqlite3_open", __FILE_NAME__, __LINE__, __PRETTY_FUNCTION__);
    CheckAndLog::NotEqualToCheck(sqlite3_exec(m_db, sql.c_str(), NULL, 0, &m_messagge_error), SQLITE_OK, "sqlite3_exec", __FILE_NAME__, __LINE__, __PRETTY_FUNCTION__);
} 




int Print(void* data, int argc, char** argv, char** azColName) 
{ 
    int i; 
    fprintf(stderr, "%s: ", (const char*)data); 
  
    for (i = 0; i < argc; i++) { 
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL"); 
    } 
  
    printf("\n"); 
    return 0; 
} 