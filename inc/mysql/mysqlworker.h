#ifndef MYSQL_WORKER_H
#define MYSQL_WORKER_H

#include <mysql/mysql.h>
#include <memory>
#include <vector>
#include <string>
#include "mysqlconfig.h"

using namespace std;


class MysqlWorker
{
private:
    MYSQL mysql;
public:
    MysqlWorker();
    MysqlWorker(const MysqlWorker&) = delete;
    MysqlWorker& operator=(const MysqlWorker&) = delete;
    ~MysqlWorker();
    bool beginWork();
    //columns表示结果应有多少列
    vector<string> executeCommand(const char *sql_cmd, int columns);
    //columns表示结果应有多少列
    vector<string> executeSelectCommand(
            const vector<string>& targets,
            const mysql_base_config::mysql_tables::MysqlTable& sql_table,
            const string& condition,
            int columns);
    //执行sql的insert语句，其中option为插入参数后的部分语句，返回执行成功与否。
    bool executeInsertCommand(
            const mysql_base_config::mysql_tables::MysqlTable& sql_table,
            const vector<string>& parameters,
            const string& option = "");
    //执行sql的delete语句，返回执行成功与否
    bool executeDeleteCommand(
            const mysql_base_config::mysql_tables::MysqlTable& sql_table,
            const string& condition);
    
};


#endif