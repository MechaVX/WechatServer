#include "friendsmessageworker.h"
#include "mysqlworker.h"
#include "mysqlconfig.h"

#include <iostream>
#include <string.h>
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
    else if (msg_stru->msg_opt == send_friend_text)
    {
        return sendTextMessage(msg_stru);
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
    string where = string("account=\'") + msg_stru->data_buf + string("\' or phone=\'") + msg_stru->data_buf + '\'';
    result = mysql->executeSelectCommand(
        {"account", "username"},
        mysql_tables::base_information,
        where,
        2);
    if (result.empty())
    {
        //如果查找失败
        msg_stru->swapSenderAndReceiver();
        msg_stru->copyDataFromString(string("fail. user ") + msg_stru->data_buf + " no " + "found.");
        
    }
    else
    {
        //如果查找成功
        auto msg_ret = TCPMessage::createTCPMessage(
            tcp_standard_message::friends,
            (int)tcp_standard_message::search_someone,
            msg_stru->sender,
            TCPMessage::server_account,
            result);
        *msg_stru = std::move(*msg_ret);
    }
}

ClientSocket FriendsMessageWorker::addFriend(TCPMessage *msg_stru)
{

    string where = "account=\'" + string(msg_stru->receiver) + '\'';
    auto ret = mysql->executeSelectCommand({ "socket" }, mysql_tables::onlined_users, where, 1);
    if (ret.empty())
    {
        //要查找的用户不在线，先储存消息
        file_worker.storeTCPMessageToFile(msg_stru->receiver, msg_stru);
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
    //新增好友关系
    mysql->executeInsertCommand(
                mysql_tables::users_relationship,
                { msg_stru->sender, msg_stru->receiver, "friend", msg_stru->receiver });
    mysql->executeInsertCommand(
                mysql_tables::users_relationship,
                { msg_stru->receiver, msg_stru->sender, "friend", msg_stru->sender });
    
    vector<string> ret = mysql->executeSelectCommand(
                { "socket" },
                mysql_tables::onlined_users,
                "account=\'" + string(msg_stru->receiver) + '\'',
                1);
    if (ret.empty())
    {
        //要查找的用户不在线，先储存消息
        file_worker.storeTCPMessageToFile(msg_stru->receiver, msg_stru);
    }
    else
    {
        //如果在线，则返回要查找的用户的socket
        ClientSocket cli_fd = stoi(ret[0]);
        return cli_fd;
    }
    return 0;
}

ClientSocket FriendsMessageWorker::sendTextMessage(TCPMessage *msg_stru)
{
    vector<string> ret = mysql->executeSelectCommand(
                { "socket" },
                mysql_tables::onlined_users,
                "account=\'" + string(msg_stru->receiver) + '\'',
                1);
    if (ret.empty())
    {
        //要查找的用户不在线，先储存消息
        file_worker.storeTCPMessageToFile(msg_stru->receiver, msg_stru);
    }
    else
    {
        //如果在线，则返回要查找的用户的socket
        ClientSocket cli_fd = stoi(ret[0]);
        return cli_fd;
    }
}