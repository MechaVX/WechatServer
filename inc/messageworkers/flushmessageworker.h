#ifndef FLUSH_MESSAGE_WORKER_H
#define FLUSH_MESSAGE_WORKER_H

#include "basemessageworker.h"

class MysqlWorker;

//用户刷新的请求，包括刷新消息，刷新好友，刷新分组，刷新设置
//用于从文件缓存中读取消息的也使用该类
class FlushMessageWorker: public BaseMessageWorker
{
private:
    MysqlWorker *mysql;
    
    list<TCPMessage> flushFriends(TCPMessage *msg_stru);
    //刷新新的消息
    list<TCPMessage> flushMessages(TCPMessage *msg_stru);
public:
    FlushMessageWorker(MysqlWorker *sql);
    list<TCPMessage> praiseMessageStruct(ClientSocket cli_fd, TCPMessage *msg_stru);
};

#endif