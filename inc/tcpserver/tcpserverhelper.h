#ifndef TCP_SERVER_HELPER
#define TCP_SERVER_HELPER

#include "globaldefine.h"
#include "settingmessageworker.h"
#include "mysqlworker.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <vector>


using namespace std;
using tcp_standard_message::TCPMessage;

class TCPServerHelper
{
private:
    MysqlWorker mysql_worker;
    SettingMessageWorker *setting_worker;

    
    //该线程用于发消息给客户端
    thread *send_thread;
    mutex send_mutex;
    condition_variable send_cond;
    list<pair<ClientSocket, vector<char> > > send_data_list;

    //该线程用于查询mysql
    thread *sql_thread;
    //socket接收到的消息通过该函数存储至此
    list<pair<ClientSocket, vector<char> > > recv_data_list;
    //recv_data_list对应的同步对象
    mutex recv_mutex;
    condition_variable sql_cond;
    //服务器关闭，则该值置false
    bool running;

    void sqlThreadLooping();
    void sendThreadLooping();

    //根据空格分隔开TCPMessage的data_buf数据
    vector<string> splitDataBySpace(int beg_index, int end_index, char *data_buf);
    //将data_buf数据反序列化还原到tcp_msg_stru中
    void praiseDataToMsgStru(TCPMessage *tcp_msg_stru, const char *recv_data);
    //分析TCPMessage中的数据，执行对应操作。返回true，表示消息需要回复给客户端
    bool convertMsgStru(TCPMessage *tcp_msg_stru);
    //sql_thread通知send_thread发送消息
    void prepareToSendMessage(ClientSocket cli_soc, const TCPMessage& msg_stru);
    //通知线程退出
    void notifyThreadsToExit();
public:
    TCPServerHelper();
    ~TCPServerHelper();
    bool serverHelperStart();
    //socket接收到的消息通过该函数存储至recv_data_list，待消息处理线程处理
    //如果成功存储返回true，不成功则表明recv_data_list被消息处理线程使用,返回false
    bool storeSocketReceiveData(ClientSocket cli_soc, const char *data_buf, int buf_len);
    //对于TCPServerEpoll所存储的list中所有消息都转移至recv_data_list，成功存储返回true，否则返回false
    bool storeSocketReceiveData(list<pair<ClientSocket, vector<char> > >& pairs_list);
    
};

#endif