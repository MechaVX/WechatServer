#include "tcpstandardmessage.h"

#include <string.h>

using std::to_string;

namespace tcp_standard_message
{
TCPMessage::TCPMessage(): data_buf(nullptr) {}

TCPMessage::~TCPMessage()
{
    if (data_buf != nullptr)
        delete data_buf;
}

TCPMessage::TCPMessage(TCPMessage&& msg_stru)
{
    this->data_len = msg_stru.data_len;
    this->msg_typ = msg_stru.msg_typ;
    this->msg_opt = msg_stru.msg_opt;
    this->data_buf = msg_stru.data_buf;
    msg_stru.msg_opt = -1;
    msg_stru.data_buf = nullptr;
}

void TCPMessage::operator=(TCPMessage&& msg_stru)
{
    if (this->data_buf != nullptr)
        delete data_buf;
    this->msg_typ = msg_stru.msg_typ;
    this->msg_opt = msg_stru.msg_opt;
    this->data_len = msg_stru.data_len;
    this->data_buf = msg_stru.data_buf;
    msg_stru.msg_typ = invalid;
    msg_stru.data_len = 0;
    msg_stru.data_buf = nullptr;
}

shared_ptr<TCPMessage> TCPMessage::createTCPMessage(MessageType msg_typ, int msg_opt, const vector<string>& strs)
{
    int size = 0;
    for (const std::string& str: strs)
    {
        size += str.length() + 1;
    }
    shared_ptr<TCPMessage> msg_stru(new TCPMessage);
    msg_stru->msg_typ = msg_typ;
    msg_stru->msg_opt = msg_opt;
    msg_stru->data_len = size;
    if (size == 0)
        return msg_stru;
    msg_stru->data_buf = new char[size];
    char *ptr = msg_stru->data_buf;
    for (const std::string& str: strs)
    {
        for (char c: str)
        {
            *ptr = c;
            ++ptr;
        }
        *ptr = ' ';
        ++ptr;
    }
    --ptr;
    *ptr = '\0';
    return msg_stru;
}

void TCPMessage::copyDataFromString(const std::string& str)
{
    if (data_buf != nullptr)
    {
        delete data_buf;
        data_buf = nullptr;
        this->data_len = 0;
    }
    if (str == "")
        return;
    this->data_len = str.length() + 1;
    data_buf = new char[data_len];
    //考虑到str可能有'\0'的成员，不用strcpy
    //strcpy(data_buf, str.data());
    char *ptr = data_buf;
    for (char c: str)
    {
        *ptr = c;
        ++ptr;
    }
    *ptr = '\0';
}

ostream& operator<<(ostream& out_, const TCPMessage& msg_stru)
{
    out_ << "msg_typ=" << msg_stru.msg_typ << ' ';
    out_ << "msg_opt=" << msg_stru.msg_opt << ' ';
    out_ << "msg_data_len=" << msg_stru.data_len << ' ';
    out_ << "msg_data:" << std::endl;
    if (msg_stru.data_buf != nullptr)
        out_ << msg_stru.data_buf << std::endl;
    return out_;
}


string TCPMessage::serializeToStdString() const
{
    string result;
    result.reserve(data_len + sizeof(TCPMessage) + 17);
    result += to_string(msg_typ) + ' ';
    result += to_string(msg_opt) + ' ';
    result += to_string(data_len) + ' ';
    for (uint32_t i = 0; i < data_len; ++i)
    {
        result.push_back(data_buf[i]);
    }
    return result;
}

int TCPMessage::createTCPMessageStream(MessageType msg_typ, int msg_opt, const vector<string>& strs, char *data_buf)
{
    int size = 0;
    for (const std::string& str: strs)
    {
        size += str.length() + 1;
    }
    string tmp = to_string(msg_typ) + ' ' + to_string(msg_opt) + ' ' + to_string(size) + ' ';
    strcpy(data_buf, tmp.data());
    char *ptr = data_buf + tmp.length();
    for (const string& str: strs)
    {
        for (char c: str)
        {
            *ptr = c;
            ++ptr;
        }
        *ptr = ' ';
        ++ptr;
    }
    --ptr;
    *ptr = '\0';
    return tmp.length() + size;
}

}