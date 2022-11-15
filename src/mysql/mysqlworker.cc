#include "mysqlworker.h"
#include "mysqlconfig.h"

#include <iostream>
#include <vector>
#include <unistd.h>

using namespace std;
using namespace mysql_base_config;

MysqlWorker::MysqlWorker()
{
    mysql_library_init(NULL, NULL, NULL);
    mysql_init(&mysql);

}

MysqlWorker::~MysqlWorker()
{
    mysql_close(&mysql);
    mysql_library_end();
}

bool MysqlWorker::beginWork()
{
    MYSQL *ret;
    for (int count = 0; count < 20; ++count)
    {
        ret = mysql_real_connect(&mysql, host, user, passwd, databasename, default_port, 0, 0);
        if (ret != NULL)
        {
            return true;
        }
        cout << "cannot connect to mysql, retry" << endl;
        sleep(1);
    }
    cout << "cconnect to mysql fail, please check if mysql server is start." << endl;
    cout << "or did you just start the process with user root?" << endl;
    return false;
}

vector<string> MysqlWorker::executeCommand(const char *sql_cmd, int columns)
{
    vector<string> result;
    if (mysql_query(&mysql, sql_cmd) != 0)
    {
        cout << mysql_error(&mysql) << endl;
        return result;
    }
    
    MYSQL_RES *sql_res = mysql_use_result(&mysql);
    //无查询结果
    if (sql_res == nullptr)
    {
        return result;
    }
    if (columns == 0)
    {
        result.push_back(mysql_error(&mysql));
        return result;
    }
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(sql_res)) != NULL)
    {
        unsigned long *lengths = mysql_fetch_lengths(sql_res);
        for (int i = 0; i < columns; ++i)
        {
            int len = lengths[i];
            result.push_back(string(len, 0));
            string& str = *result.rbegin();
            for (int j = 0; j < len; ++j)
            {
                str[j] = row[i][j];
            }
        }
    }
    mysql_free_result(sql_res);
    return std::move(result);
}

vector<string> MysqlWorker::executeSelectCommand(
        const vector<string>& targets,
        const mysql_base_config::mysql_tables::MysqlTable& sql_table,
        const string& condition,
        int columns)
{
    vector<string> result;
    if (targets.empty())
        return result;
    string parameters;
    for (int i = 0, length = targets.size() - 1; i < length; ++i)
    {
        parameters += targets[i] + ',';
    }
    parameters += *targets.rbegin();
    string cmd = "select " + parameters + " from " + sql_table.name + " where " + condition;
    result = executeCommand(cmd.data(), columns);
    return result;
}

bool MysqlWorker::executeInsertCommand(
        const mysql_base_config::mysql_tables::MysqlTable& sql_table,
        const vector<string>& parameters,
        const string& option)
{
    if (parameters.empty())
        return false;
    int sub = sql_table.columns_name.size() - parameters.size();
    if (sub < 0)
    {
        cout << "too many parameters for table " << sql_table.name << endl;
        cout << "parameter:\n";
        for (auto& str: parameters)
            cout << str << endl;
        return false;
    }
    string sql_cmd;
    sql_cmd = "insert into " + sql_table.name + " (";
    for (const string& column: sql_table.columns_name)
    {
        sql_cmd += column + ',';
    }
    *sql_cmd.rbegin() = ')';
    sql_cmd += " values(\'";
    for (auto& str: parameters)
    {
        sql_cmd += str + "\',\'";
    }
    sql_cmd.pop_back();
    //补齐null值，否则这版本的mysql似乎不通过语法
    for (; sub > 0; --sub)
    {
        sql_cmd += "null,";
    }
    *sql_cmd.rbegin() = ')';
    if (option != "")
        sql_cmd += ' ' + option;
    return executeCommand(sql_cmd.data(), 0).empty();
}

bool MysqlWorker::executeDeleteCommand(
        const mysql_base_config::mysql_tables::MysqlTable& sql_table,
        const string& condition)
{
    string sql_cmd = "delete from " + sql_table.name + " where " + condition;
    auto ret = executeCommand(sql_cmd.data(), 0);
    return true;
}