#include "settingmessageworker.h"
#include "mysqlworker.h"
#include "mysqlconfig.h"

#include <iostream>
#include <ctime>

using namespace mysql_base_config;

SettingMessageWorker::SettingMessageWorker(MysqlWorker *sql_worker): mysql(sql_worker) {}


ClientSocket SettingMessageWorker::praiseMessageStruct(ClientSocket cli_fd, TCPMessage *msg_stru)
{
    string ret;
    if (msg_stru->msg_opt == user_register)
    {
        ret = userRegister(msg_stru);
        //结构体拷贝返回结果，作为返回给客户端的信息
        msg_stru->copyDataFromString(ret);
    }
    else if (msg_stru->msg_opt == user_login)
    {
        ret = userLogin(msg_stru, cli_fd);
        //结构体拷贝返回结果，作为返回给客户端的信息
        msg_stru->copyDataFromString(ret);
    }
    else if (msg_stru->msg_opt == user_logout)
    {
        ret = userLogout(msg_stru);
        return 0;
    }
    return cli_fd;
}

string SettingMessageWorker::userRegister(TCPMessage *msg_stru)
{
    //参数第一个是手机号，第二个是密码
    vector<string> parameters = this->splitDataBySpace(msg_stru->data_buf);
    string sql_cmd = "select count(phone) from " + mysql_tables::base_information.name + " where phone=\'"
        + parameters[0] + '\'';
    auto result = mysql->executeCommand(sql_cmd.data(), 1);
    if (result[0] != "0")
    {
        return string("fail, the phone has been used to register.");
    }
    static int base_account = 1643616079;
    //该数字加上base_account，等于注册成功的账号
    sql_cmd = "select count(account) from " + mysql_tables::base_information.name;
    auto ret_cnt = mysql->executeCommand(sql_cmd.data(), 1);
    int account_count = stoi(ret_cnt[0]);
    string new_account = to_string(account_count + base_account);
    //新插入注册的用户
    mysql->executeInsertCommand(mysql_tables::base_information, { new_account, parameters[1], parameters[0], new_account});
    //为新用户创建文件夹
    file_worker.createNewAccountDir(new_account);
    return new_account;
}

string SettingMessageWorker::userLogin(TCPMessage *msg_stru, ClientSocket cli_fd)
{
    //参数第一个是帐号，第二个是密码
    vector<string> parameters = this->splitDataBySpace(msg_stru->data_buf);
    string sql_cmd = "select count(account) from " + mysql_tables::base_information.name + " where account=\'"
        + parameters[0] + "\' and password=\'" + parameters[1] + '\'';
    auto result = mysql->executeCommand(sql_cmd.data(), 1);
    if (result[0] == "0")
        return string("fail, incorrect account or password.");
    mysql->executeDeleteCommand(mysql_tables::onlined_users, string("account=\'") + parameters[0] + '\'');
    mysql->executeInsertCommand(mysql_tables::onlined_users, { parameters[0] , to_string(cli_fd) });
    return string("success.");
}

string SettingMessageWorker::userLogout(TCPMessage *msg_stru)
{
    string data = msg_stru->data_buf;
    string tmp;
    if (data.length() == 10)
    {
        tmp = "account=\'";
    }
    else
    {
        tmp = "socket=\'";
    }
    mysql->executeDeleteCommand(mysql_tables::onlined_users, tmp + data + '\'');
    return string();
}
