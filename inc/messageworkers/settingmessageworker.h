#ifndef SETTING_MESSAGE_WORKER_H
#define SETTING_MESSAGE_WORKER_H

#include "basemessageworker.h"
#include "tcpstandardmessage.h"

class MysqlWorker;

using namespace std;
using namespace tcp_standard_message;

//该类专门处理tcp_standard_message::message_type为setting的请求
class SettingMessageWorker: public BaseMessageWorker
{
private:
    MysqlWorker *mysql;
    //规定返回值带fail表示执行失败，
    string userRegister(TCPMessage *msg_stru);
    string userLogin(TCPMessage *msg_stru);
public:
    SettingMessageWorker(MysqlWorker *sql_worker);

    void praiseMessageStruct(TCPMessage *msg_stru);
};

#endif