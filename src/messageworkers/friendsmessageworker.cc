#include "friendsmessageworker.h"
#include "mysqlworker.h"
#include "mysqlconfig.h"

#include <iostream>
using namespace mysql_base_config;

FriendsMessageWorker::FriendsMessageWorker(MysqlWorker *sql_worker): mysql(sql_worker) {}

ClientSocket FriendsMessageWorker::praiseMessageStruct(ClientSocket cli_fd, TCPMessage *msg_stru)
{
    if (msg_stru->msg_opt == search_someone)
    {
        searchUser(msg_stru);
        return cli_fd;
    }
    else if (msg_stru->msg_opt == add_friend)
    {
        return addFriend(msg_stru);
    }
    else if (msg_stru->msg_opt == agree_add_friend)
    {
        return agreeAddFriend(msg_stru);
    }
    else
    {
        cout << "FriendsMessageWorker::praiseMessageStruct: unknow msg_opt";
        cout << msg_stru->msg_opt << endl;
        return 0;
    }
    return 0;
}

void FriendsMessageWorker::searchUser(TCPMessage *msg_stru)
{
    vector<string> result;
    string tmp;
    if (msg_stru->data_len == 11)
        tmp = "account=\'";
    else if (msg_stru->data_len == 12)
        tmp = "phone=\'";
    result = mysql->executeSelectCommand(
        {"account", "username"},
        mysql_tables::base_information,
        tmp + msg_stru->data_buf + '\'',
        2);
    if (result.empty())
    {
        //如果查找失败
        tmp = string("fail. user ") + msg_stru->data_buf + " no " + "found.";
        msg_stru->copyDataFromString(tmp);
    }
    else
    {
        //如果查找成功
        auto msg_ret = TCPMessage::createTCPMessage(tcp_standard_message::friends,
            tcp_standard_message::search_someone,
            result);
        *msg_stru = std::move(*msg_ret);
    }
}

ClientSocket FriendsMessageWorker::addFriend(TCPMessage *msg_stru)
{

    //result[0]为新好友账户，result[1]为本人账户，其余为申请信息留言
    vector<string> result = this->splitDataBySpace(msg_stru->data_buf);
    if (result.size() < 2)
    {
        cout << "client message format error:\n" << endl;
        cout << msg_stru->data_buf << endl;
        return 0;
    }
    string where = "account=\'" + result[0] + '\'';
    auto ret = mysql->executeSelectCommand({ "socket" }, mysql_tables::onlined_users, where, 1);
    if (ret.empty())
    {
        //要查找的用户不在线，先储存消息
        file_worker.storeTCPMessageToFile(result[0], msg_stru);
    }
    else
    {
        //如果在线，则返回要查找的用户的socket
        ClientSocket cli_fd = stoi(ret[0]);
        return cli_fd;
    }
    return 0;
}

ClientSocket FriendsMessageWorker::agreeAddFriend(TCPMessage *msg_stru)
{
    //result[0]为响应用户账号，result[1]为请求用户账号
    vector<string> result = this->splitDataBySpace(msg_stru->data_buf);
    
    //新增好友关系
    mysql->executeInsertCommand(
                mysql_tables::users_relationship,
                { result[0], result[1], "friend", result[1] });
    mysql->executeInsertCommand(
                mysql_tables::users_relationship,
                { result[1], result[0], "friend", result[0] });
    
    vector<string> ret = mysql->executeSelectCommand(
                { "socket" },
                mysql_tables::onlined_users,
                "account=\'" + result[1] + '\'',
                1);
    if (ret.empty())
    {
        //要查找的用户不在线，先储存消息
        file_worker.storeTCPMessageToFile(result[0], msg_stru);
    }
    else
    {
        //如果在线，则返回要查找的用户的socket
        ClientSocket cli_fd = stoi(ret[0]);
        return cli_fd;
    }
    return 0;
}