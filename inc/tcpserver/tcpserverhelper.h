#ifndef TCP_SERVER_HELPER
#define TCP_SERVER_HELPER

#include "globaldefine.h"
#include "settingmessageworker.h"
#include "friendsmessageworker.h"
#include "flushmessageworker.h"
#include "mysqlworker.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <vector>
#include <unordered_map>


using namespace std;
using tcp_standard_message::TCPMessage;

class TCPServerHelper
{
private:
    MysqlWorker mysql_worker;
    SettingMessageWorker *setting_worker;
    FriendsMessageWorker *friends_worker;
    FlushMessageWorker *flush_worker;
    //该线程用于发消息给客户端
    thread *send_thread;
    mutex send_mutex;
    condition_variable send_cond;
    list<pair<ClientSocket, vector<char> > > send_data_list;

    //该线程用于查询mysql
    thread *sql_thread;
    //socket接收到的消息通过该函数存储至此
    list<pair<ClientSocket, vector<char> > > recv_data_list;
    //recv_data_list和sql_commands对应的同步对象
    mutex recv_mutex;
    condition_variable sql_cond;
    //这部分数据是从缓存文件读取的，也需要发送出去
    list<TCPMessage> cache_data_list;

    

    //为了解决TCP粘包的问题，如果判断某个socket的消息未发送完毕，先将消息使用string类储存至此
    unordered_map<ClientSocket, string> incomplete_data;
    mutex incomplete_data_mutex;

    //服务器关闭，则该值置false
    bool running;

    void sqlThreadLooping();
    void sendThreadLooping();

    //根据空格分隔开TCPMessage的data_buf数据
    vector<string> splitDataBySpace(int beg_index, int end_index, char *data_buf);
    //将data_buf数据反序列化还原到TCPMessage中返回
    list<TCPMessage> praiseDataToMsgStru(ClientSocket cli_fd, vector<char>& data_buf);
    //分析TCPMessage中的数据，执行对应操作。返回值表示要回复的客户端socket，返回0表示不需要回复
    ClientSocket convertMsgStru(ClientSocket cli_fd, TCPMessage *tcp_msg_stru);
    //sql_thread通知send_thread发送消息
    void prepareToSendMessage(ClientSocket cli_soc, const TCPMessage& msg_stru);
    //为提高效率，该函数提供方法将整个链表中的消息一次性转移
    void prepareToSendMessage(ClientSocket cli_soc, const list<TCPMessage>& msg_list);
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