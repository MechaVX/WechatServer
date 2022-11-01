#include "settingmessageworker.h"
#include "mysqlworker.h"
#include "mysqlconfig.h"

#include <iostream>
#include <ctime>

using namespace mysql_base_config;

SettingMessageWorker::SettingMessageWorker(MysqlWorker *sql_worker): mysql(sql_worker) {}


void SettingMessageWorker::praiseMessageStruct(TCPMessage *msg_stru)
{
    string ret;
    if (msg_stru->msg_opt == user_register)
    {
        ret = userRegister(msg_stru);
    }
    else if (msg_stru->msg_opt == user_login)
    {
        ret = userLogin(msg_stru);
    }
    //结构体拷贝返回结果，作为返回给客户端的信息
    msg_stru->copyDataFromString(ret);
}

string SettingMessageWorker::userRegister(TCPMessage *msg_stru)
{
    //参数第一个是手机号，第二个是密码
    vector<string> parameters = this->splitDataBySpace(msg_stru->data_buf);
    string sql_cmd = "select count(phone) from " + string(tables_name::base_info) + " where phone=\'"
        + parameters[0] + '\'';
    auto result = mysql->executeCommand(sql_cmd.data(), 1);
    if (result[0] != "0")
    {
        return string("fail, the phone has been used to register.");
    }
    static int base_account = 1643616079;
    //该数字加上base_account，等于注册成功的账号
    sql_cmd = "select count(account) from " + string(tables_name::base_info);
    auto ret_cnt = mysql->executeCommand(sql_cmd.data(), 1);
    int account_count = stoi(ret_cnt[0]);
    string new_account = to_string(account_count + base_account);
    //新插入注册的用户
    mysql->executeInsertCommand(tables_name::base_info, { new_account, parameters[1], parameters[0] });
    return new_account;
}

string SettingMessageWorker::userLogin(TCPMessage *msg_stru)
{
    //参数第一个是帐号，第二个是密码
    vector<string> parameters = this->splitDataBySpace(msg_stru->data_buf);
    string sql_cmd = "select count(account) from " + string(tables_name::base_info) + " where account=\'"
        + parameters[0] + "\' and password=\'" + parameters[1] + '\'';
    auto result = mysql->executeCommand(sql_cmd.data(), 1);
    if (result[0] == "0")
        return string("fail, incorrect account or password.");
    mysql->executeInsertCommand(tables_name::onlined, { parameters[0] });
    return string("success.");
}