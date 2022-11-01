#include "tcpserverhelper.h"
#include "mysqlworker.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <string.h>



TCPServerHelper::TCPServerHelper(): send_thread(nullptr), sql_thread(nullptr), running(false)
{
    setting_worker = new SettingMessageWorker(&mysql_worker);
}

TCPServerHelper::~TCPServerHelper()
{
    
    notifyThreadsToExit();
    if (send_thread != nullptr)
        delete send_thread;
    if (sql_thread != nullptr)
        delete sql_thread;
    delete setting_worker;
    
}

void TCPServerHelper::notifyThreadsToExit()
{
    this->running = false;

    {
        unique_lock<mutex> lock(send_mutex);
        send_cond.notify_all();
    }
    {
        unique_lock<mutex> lock(recv_mutex);
        sql_cond.notify_all();
    }
    
    if (send_thread != nullptr && send_thread->joinable())
        send_thread->join();
    if (sql_thread != nullptr && sql_thread->joinable())
        sql_thread->join();
}

bool TCPServerHelper::serverHelperStart()
{
    bool success = mysql_worker.beginWork();
    if (!success)
        return false;
    this->running = true;
    send_thread = new thread(&TCPServerHelper::sendThreadLooping, this);
    sql_thread = new thread(&TCPServerHelper::sqlThreadLooping, this);
    return true;
}


void TCPServerHelper::prepareToSendMessage(ClientSocket cli_soc, const TCPMessage& msg_stru)
{
    int length = msg_stru.data_len;
    string str;
    str.reserve(length + sizeof(TCPMessage) + 17);
    str += to_string(msg_stru.msg_typ) + ' ';
    str += to_string(msg_stru.msg_opt) + ' ';
    str += to_string(length) + ' ';
    for (int i = 0; i < length; ++i)
    {
        str.push_back(msg_stru.data_buf[i]);
    }
    vector<char> tmp;
    tmp.reserve(str.length());
    for (char c: str)
    {
        tmp.push_back(c);
    }
    unique_lock<mutex> lock(send_mutex);
    send_data_list.emplace_back(cli_soc, std::move(tmp));
    send_cond.notify_one();
}

void TCPServerHelper::sendThreadLooping()
{
    while (1)
    {
        unique_lock<mutex> lock(send_mutex);
        send_cond.wait(lock, [this]()
        {
            return !running || !send_data_list.empty();
        });
        if (!running)
            break;
        for (auto it = send_data_list.begin(); it != send_data_list.end();)
        {
            send(it->first, it->second.data(), it->second.size(), 0);
            send_data_list.erase(it++);
        }
    }
}

void TCPServerHelper::sqlThreadLooping()
{
    while (1)
    {
        unique_lock<mutex> lock(recv_mutex);
        sql_cond.wait(lock, [this]()
        {
            return !running || !recv_data_list.empty();
        });
        if (!running)
            break;
        for (auto it = recv_data_list.begin(); it != recv_data_list.end();)
        {
            TCPMessage msg_stru;
            praiseDataToMsgStru(&msg_stru, it->second.data());
            if (convertMsgStru(&msg_stru))
            {
                prepareToSendMessage(it->first, msg_stru);
            }
            recv_data_list.erase(it++);
        }
    }
}

bool TCPServerHelper::storeSocketReceiveData(ClientSocket cli_soc, const char *data_buf, int buf_len)
{
    unique_lock<mutex> lock(recv_mutex, try_to_lock);
    //如果不能获得锁，离开返回false，避免主线程持续等待
    if (!lock.owns_lock())
        return false;
    vector<char> tmp;
    tmp.reserve(buf_len);
    for (int i = 0; i < buf_len; ++i)
    {
        tmp.push_back(data_buf[i]);
    }
    recv_data_list.emplace_back(cli_soc, std::move(tmp));
    sql_cond.notify_one();
    return true;
}

bool TCPServerHelper::storeSocketReceiveData(list<pair<ClientSocket, vector<char> > >& pairs_list)
{
    unique_lock<mutex> lock(recv_mutex, try_to_lock);
    if (!lock.owns_lock())
        return false;
    for (auto& pair_: pairs_list)
    {
        recv_data_list.push_back(std::move(pair_));
    }
    sql_cond.notify_one();
    return true;
}

void TCPServerHelper::praiseDataToMsgStru(
    TCPMessage *tcp_msg_stru, const char *recv_data)
{
    int end_index = 0;
    int beg_index = 0;
    string str_data = recv_data;
    while (str_data[end_index] != ' ')
        ++end_index;
    tcp_msg_stru->msg_typ = (tcp_standard_message::message_type)stoi
        (str_data.substr(beg_index, end_index - beg_index));
    
    ++end_index;
    beg_index = end_index;
    while (str_data[end_index] != ' ')
        ++end_index;
    tcp_msg_stru->msg_opt = stoi(str_data.substr(beg_index, end_index - beg_index));
    
    ++end_index;
    beg_index = end_index;
    while (str_data[end_index] != ' ')
        ++end_index;
    tcp_msg_stru->data_len = stoi(str_data.substr(beg_index, end_index - beg_index));

    tcp_msg_stru->data_buf = new char[tcp_msg_stru->data_len];
    for (int index = end_index + 1, count = 0; count < tcp_msg_stru->data_len; ++count, ++index)
    {
        tcp_msg_stru->data_buf[count] = recv_data[index];
    }
}

bool TCPServerHelper::convertMsgStru(TCPMessage *tcp_msg_stru)
{
    if (tcp_msg_stru->msg_typ == tcp_standard_message::setting)
    {
        setting_worker->praiseMessageStruct(tcp_msg_stru);
        return true;
    }
    return false;
}