#include "tcpserverhelper.h"
#include "mysqlworker.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <string.h>
#include <queue>


TCPServerHelper::TCPServerHelper(): send_thread(nullptr), sql_thread(nullptr), running(false)
{
    setting_worker = new SettingMessageWorker(&mysql_worker);
    friends_worker = new FriendsMessageWorker(&mysql_worker);
    flush_worker = new FlushMessageWorker(&mysql_worker);
}

TCPServerHelper::~TCPServerHelper()
{
    notifyThreadsToExit();
    if (send_thread != nullptr)
        delete send_thread;
    if (sql_thread != nullptr)
        delete sql_thread;
    delete setting_worker;
    delete friends_worker;
    delete flush_worker;
    
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
    string str = msg_stru.serializeToStdString();
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

void TCPServerHelper::prepareToSendMessage(ClientSocket cli_soc, const list<TCPMessage>& msg_list)
{
    if (msg_list.empty())
        return;
    string str;
    unique_lock<mutex> lock(send_mutex);
    auto last_it = msg_list.end();
    --last_it;
    for (auto it = msg_list.begin(); it != msg_list.end(); ++it)
    {
        str = it->serializeToStdString();
        vector<char> tmp;
        tmp.reserve(str.length());
        for (char c: str)
        {
            tmp.push_back(c);
        }
        send_data_list.emplace_back(cli_soc, std::move(tmp));
        if (it == last_it)
            send_cond.notify_one();
        
    }
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
            list<TCPMessage > msg_strus = praiseDataToMsgStru(it->first, it->second);
            for (TCPMessage& msg_stru: msg_strus)
            {
                ClientSocket ret_fd = convertMsgStru(it->first, &msg_stru);
                //??????0??????msg_stru????????????ret_fd
                if (ret_fd > 0)
                {
                    prepareToSendMessage(ret_fd, msg_stru);
                }
                else if (ret_fd < 0)
                {
                    //????????????
                    removeSocketData(-ret_fd);
                }
            }
            prepareToSendMessage(it->first, cache_data_list);
            cache_data_list.clear();
            recv_data_list.erase(it++);
        }
    }
}

bool TCPServerHelper::storeSocketReceiveData(ClientSocket cli_soc, const char *data_buf, int buf_len)
{
    unique_lock<mutex> lock(recv_mutex, try_to_lock);
    //????????????????????????????????????false??????????????????????????????
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

list<TCPMessage> TCPServerHelper::praiseDataToMsgStru(ClientSocket cli_fd, vector<char>& recv_data)
{
    list<TCPMessage> result;
    //???????????????????????????????????????
    queue<char> datas;
    auto it = incomplete_data.find(cli_fd);
    if (it != incomplete_data.end())
    {
        string& str = it->second;
        for (char c: str)
            datas.push(c);
        incomplete_data.erase(it);
    }
    for (char c: recv_data)
    {
        datas.push(c);
    }

    while (!datas.empty())
    {
        
        string strs[6];
        int index = 0;
        //??????????????????????????????????????????false
        auto analizeDatas = [&]()
        {
            char c;
            while (!datas.empty())
            {
                c = datas.front();
                datas.pop();
                if (c == ' ')
                    break;
                strs[index].push_back(c);
            }
            if (c != ' ')
            {
                //????????????????????????????????????incomplete_data??????????????????????????????????????????
                string& str_buf = this->incomplete_data[cli_fd];
                for (int i = 0; i < index; ++i)
                {
                    str_buf += strs[i] + ' ';
                }
                for (char c: strs[index])
                    str_buf.push_back(c);
                return false;
            }
            return true;
        };

        TCPMessage tcp_msg_stru;

        if (!analizeDatas())
            return std::move(result);
        tcp_msg_stru.timestamp = (tcp_standard_message::MessageType)stoi(strs[index]);
        ++index;


        if (!analizeDatas())
            return std::move(result);
        tcp_msg_stru.msg_typ = (tcp_standard_message::MessageType)stoi(strs[index]);
        ++index;


        if (!analizeDatas())
            return std::move(result);
        tcp_msg_stru.msg_opt = stoi(strs[index]);
        ++index;


        if (!analizeDatas())
            return std::move(result);
        for (int i = 0; i < ACCOUNT_LEN; ++i)
            tcp_msg_stru.sender[i] = strs[index][i];
        ++index;


        if (!analizeDatas())
            return std::move(result);
        for (int i = 0; i < ACCOUNT_LEN; ++i)
            tcp_msg_stru.receiver[i] = strs[index][i];
        ++index;

        if (!analizeDatas())
            return std::move(result);
        tcp_msg_stru.data_len = stoi(strs[index]);

        //????????????????????????????????????incomplete_data??????????????????????????????????????????
        if (datas.size() < tcp_msg_stru.data_len)
        {
            string& str_buf = incomplete_data[cli_fd];
            for (string& str: strs)
            {
                str_buf += str + ' ';
            }
            while (!datas.empty())
            {
                str_buf.push_back(datas.front());
                
                datas.pop();
            }
            
            return std::move(result);
        }
        if (tcp_msg_stru.data_len > 0)
        {
            
            tcp_msg_stru.data_buf = new char[tcp_msg_stru.data_len];
            for (uint32_t i = 0; i < tcp_msg_stru.data_len; ++i)
            {
                tcp_msg_stru.data_buf[i] = datas.front();
                datas.pop();
            }
        }
        result.push_back(std::move(tcp_msg_stru));
    }
    return std::move(result);
}

/*
list<TCPMessage> TCPServerHelper::praiseDataToMsgStru(ClientSocket cli_fd, vector<char>& recv_data)
{
    list<TCPMessage> result;
    int start_index = 0;
    //???????????????????????????????????????
    while (true)
    {
        int end_index = 0;
        int beg_index = 0;
        string str_data;
        //?????????????????????incomplete_data??????????????????
        if (start_index == 0)
        {
            //???????????????????????????????????????????????????
            unique_lock<mutex> lock(incomplete_data_mutex);
            auto it = this->incomplete_data.find(cli_fd);
            if (it != incomplete_data.end())
            {
                
                string& data = it->second;
                vector<char> tmp;
                tmp.reserve(data.length() + recv_data.size());
                int size = data.length();
                for (char c: data)
                {
                    tmp.push_back(c);
                }
                for (char c: recv_data)
                {
                    tmp.push_back(c);
                }
                incomplete_data.erase(it);
                recv_data.swap(tmp);
            }
        }
        //??????????????????
        int size = recv_data.size();
        str_data.reserve(size + str_data.size() - start_index);
        for (int i = start_index; i < size; ++i)
        {
            str_data.push_back(recv_data[i]);
        }
        size = str_data.size();
        while (str_data[end_index] != ' ' && end_index < size)
            ++end_index;
        //????????????????????????????????????incomplete_data??????????????????????????????????????????
        if (end_index == size)
        {
            unique_lock<mutex> lock(incomplete_data_mutex);
            incomplete_data.insert(pair<ClientSocket, string >(cli_fd, str_data));
            return std::move(result);
        }
        string tmp;
        TCPMessage tcp_msg_stru;
        tmp = str_data.substr(beg_index, end_index - beg_index);
        tcp_msg_stru.msg_typ = (tcp_standard_message::MessageType)stoi(tmp);
        
        ++end_index;
        beg_index = end_index;
        while (str_data[end_index] != ' ' && end_index < size)
            ++end_index;
        if (end_index == size)
        {
            unique_lock<mutex> lock(incomplete_data_mutex);
            incomplete_data.insert(pair<ClientSocket, string >(cli_fd, str_data));
            return std::move(result);
        }
        tmp = str_data.substr(beg_index, end_index - beg_index);
        tcp_msg_stru.msg_opt = (tcp_standard_message::MessageType)stoi(tmp);
        
        ++end_index;
        beg_index = end_index;
        while (str_data[end_index] != ' ' && end_index < size)
            ++end_index;
        if (end_index == size)
        {
            unique_lock<mutex> lock(incomplete_data_mutex);
            incomplete_data.insert(pair<ClientSocket, string >(cli_fd, str_data));
            return std::move(result);
        }
        tmp = str_data.substr(beg_index, end_index - beg_index);
        tcp_msg_stru.data_len = (tcp_standard_message::MessageType)stoi(tmp);

        ++end_index;
        int len_sub = size - end_index;
        if (len_sub < tcp_msg_stru.data_len)
        {
            //????????????
            unique_lock<mutex> lock(incomplete_data_mutex);
            incomplete_data.insert(pair<ClientSocket, string >(cli_fd, str_data));
            return std::move(result);
        }
        if (tcp_msg_stru.data_len > 0)
        {
            tcp_msg_stru.data_buf = new char[tcp_msg_stru.data_len];
            for (int count = 0; count < tcp_msg_stru.data_len; ++count, ++end_index)
            {
                tcp_msg_stru.data_buf[count] = str_data[end_index];
            }
        }
        int len = tcp_msg_stru.data_len;
        result.push_back(std::move(tcp_msg_stru));
        
        if (len_sub > len)
        {
            //???????????????????????????????????????????????????
            start_index += end_index;
        }
        else
            break;
    }
    return std::move(result);
}
*/

ClientSocket TCPServerHelper::convertMsgStru(ClientSocket cli_fd, TCPMessage *tcp_msg_stru)
{
    
    if (tcp_msg_stru->msg_typ == tcp_standard_message::setting)
    {
        return setting_worker->praiseMessageStruct(cli_fd, tcp_msg_stru);
    }
    else if (tcp_msg_stru->msg_typ == tcp_standard_message::friends)
    {
        return friends_worker->praiseMessageStruct(cli_fd, tcp_msg_stru);
    }
    else if (tcp_msg_stru->msg_typ == tcp_standard_message::flush)
    {
    
        list<TCPMessage> msg_list = flush_worker->praiseMessageStruct(cli_fd, tcp_msg_stru);
        for (auto& msg: msg_list)
        {
            cache_data_list.push_back(std::move(msg));
        }
        //tcp_msg_stru????????????????????????????????????????????????????????????msg_list
        return 0;
    }
    return 0;
}

void TCPServerHelper::removeSocketData(ClientSocket cli_fd)
{
    auto it = incomplete_data.find(cli_fd);
    if (it != incomplete_data.end())
        incomplete_data.erase(it);
}