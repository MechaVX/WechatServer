#include "flushmessageworker.h"
#include "mysqlconfig.h"
#include "mysqlworker.h"
#include "fileworker.h"

#include <iostream>
using namespace mysql_base_config;
using namespace tcp_standard_message;

FlushMessageWorker::FlushMessageWorker(MysqlWorker *sql): mysql(sql) {}

list<TCPMessage> FlushMessageWorker::praiseMessageStruct(ClientSocket cli_fd, TCPMessage *msg_stru)
{
    if (msg_stru->msg_opt == flush_message)
    {
        return flushMessages(msg_stru);
    }
    else if (msg_stru->msg_opt == flush_friends)
    {
        return flushFriends(msg_stru);
    }
    return {};
}

list<TCPMessage> FlushMessageWorker::flushMessages(TCPMessage *msg_stru)
{
    string account = msg_stru->data_buf;
    //储存需要发送的消息
    list<TCPMessage> msg_list;
    file_worker.readTCPMessageFromFile(account, msg_list);
    
    
    return std::move(msg_list);
}

list<TCPMessage> FlushMessageWorker::flushFriends(TCPMessage *msg_stru)
{
    //账号为msg_stru->data_buf全部
    string account = msg_stru->data_buf;
    vector<string> ret = mysql->executeSelectCommand(
                { "friend_account", "friend_label", "friend_nickname" },
                mysql_tables::users_relationship,
                "account=\'" + account + '\'',
                3);
    vector<string> result;
    if (!ret.empty())
    {
        vector<string> username(ret.size() / 3);
        for (int size = username.size(), i = 0, j = 0; j < size; i += 3, ++j)
        {
            vector<string> tmp = mysql->executeSelectCommand(
                    { "username" },
                    mysql_tables::base_information,
                    "account=\'" + ret[i] + '\'',
                    1);
            username[j].swap(tmp[0]);
        }
        
        result.resize(ret.size() + username.size());
        for (int size = username.size(), i = 0, j = 0, k = 0; i < size;)
        {
            result[k++].swap(ret[j++]);
            result[k++].swap(ret[j++]);
            result[k++].swap(ret[j++]);
            result[k++].swap(username[i++]);
        }
    }
    auto ret_msg = TCPMessage::createTCPMessage(
                    tcp_standard_message::flush,
                    flush_friends,
                    result);
    list<TCPMessage> ret_list(1);
    ret_list.back() = std::move(*ret_msg);
    return std::move(ret_list);
}
