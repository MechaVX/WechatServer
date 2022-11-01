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

    if (columns == 0)
    {
        result.push_back(mysql_error(&mysql));
        return std::move(result);
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

bool MysqlWorker::executeInsertCommand(const string& sql_table, const vector<string>& parameters, const string& option)
{
    if (parameters.empty())
        return false;
    string sql_cmd;
    sql_cmd = "insert into " + sql_table + " values(\'";
    int len = parameters.size() - 1;
    for (int i = 0; i < len; ++i)
    {
        sql_cmd += parameters[i] + "\',\'";
    }
    sql_cmd += parameters[len] + "\') " + option;
    return executeCommand(sql_cmd.data(), 0).empty();
}