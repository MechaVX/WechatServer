#ifndef MYSQL_WORKER_H
#define MYSQL_WORKER_H

#include <mysql/mysql.h>
#include <memory>
#include <vector>
#include <string>

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
    
    //执行sql的insert语句，其中option为插入参数后的部分语句，返回执行成功与否。
    bool executeInsertCommand(const string& sql_table, const vector<string>& parameters, const string& option = "");
};


#endif