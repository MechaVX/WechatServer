#ifndef FRIENDS_MESSAGE_WORKER_H
#define FRIENDS_MESSAGE_WORKER_H

#include "basemessageworker.h"
#include "tcpstandardmessage.h"

class MysqlWorker;

using namespace std;
using namespace tcp_standard_message;

class FriendsMessageWorker: public BaseMessageWorker
{
private:
    MysqlWorker *mysql;
    //成功msg_stru->data_buf="account nick_name";
    //失败msg_stru->data_buf="fail. user account||phone no found.";
    void searchUser(TCPMessage *msg_stru);
    ClientSocket addFriend(TCPMessage *msg_stru);
    ClientSocket agreeAddFriend(TCPMessage *msg_stru);
public:
    FriendsMessageWorker(MysqlWorker *sql_worker);
    //返回值表示要回复的客户端socket，返回0表示不需要回复
    ClientSocket praiseMessageStruct(ClientSocket cli_fd, TCPMessage *msg_stru);
};

#endif