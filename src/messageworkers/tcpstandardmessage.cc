#include "tcpstandardmessage.h"

#include <string.h>


using std::to_string;

namespace tcp_standard_message
{

const string TCPMessage::server_account = "00000000000";

TCPMessage::TCPMessage(): data_buf(nullptr)
{
    this->timestamp = GetNowTimestamp();
    this->sender[ACCOUNT_LEN] = '\0';
    this->receiver[ACCOUNT_LEN] = '\0';
}

TCPMessage::~TCPMessage()
{
    if (data_buf != nullptr)
        delete data_buf;
}

TCPMessage::TCPMessage(TCPMessage&& msg_stru)
{
    this->timestamp = msg_stru.timestamp;
    this->data_len = msg_stru.data_len;
    this->msg_typ = msg_stru.msg_typ;
    this->msg_opt = msg_stru.msg_opt;
    strcpy(this->sender, msg_stru.sender);
    strcpy(this->receiver, msg_stru.receiver);
    this->data_buf = msg_stru.data_buf;
    msg_stru.msg_opt = -1;
    msg_stru.data_buf = nullptr;
}

void TCPMessage::operator=(TCPMessage&& msg_stru)
{
    if (this->data_buf != nullptr)
        delete data_buf;
    this->timestamp = msg_stru.timestamp;
    this->msg_typ = msg_stru.msg_typ;
    this->msg_opt = msg_stru.msg_opt;
    strcpy(this->sender, msg_stru.sender);
    strcpy(this->receiver, msg_stru.receiver);
    this->data_len = msg_stru.data_len;
    this->data_buf = msg_stru.data_buf;
    msg_stru.msg_typ = invalid;
    msg_stru.data_len = 0;
    msg_stru.data_buf = nullptr;
}

shared_ptr<TCPMessage> TCPMessage::createTCPMessage(
                MessageType msg_typ,
                int msg_opt,
                const string& receiver,
                const string& sender,
                const std::vector<std::string>& strs)
{
    if (receiver.length() != 11 || sender.length() != 11)
    {
        throw 1;
        return shared_ptr<TCPMessage>();
    }
    int size = 0;
    for (const std::string& str: strs)
    {
        size += str.length() + 1;
    }
    shared_ptr<TCPMessage> msg_stru(new TCPMessage);
    msg_stru->msg_typ = msg_typ;
    msg_stru->msg_opt = msg_opt;
    strcpy(msg_stru->sender, sender.data());
    strcpy(msg_stru->receiver, receiver.data());
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
    out_ << "timestamp=" << msg_stru.timestamp << ' ';
    out_ << "msg_typ=" << msg_stru.msg_typ << ' ';
    out_ << "msg_opt=" << msg_stru.msg_opt << ' ';
    out_ << "sender=" << msg_stru.sender << ' ';
    out_ << "receiver=" << msg_stru.receiver << ' ';
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
    result += to_string(timestamp) + ' ';
    result += to_string(msg_typ) + ' ';
    result += to_string(msg_opt) + ' ';
    for (char c: sender)
        result.push_back(c);
    result.push_back(' ');
    for (char c: receiver)
        result.push_back(c);
    result.push_back(' ');
    result += to_string(data_len) + ' ';
    for (uint32_t i = 0; i < data_len; ++i)
    {
        result.push_back(data_buf[i]);
    }
    return result;
}

void TCPMessage::swapSenderAndReceiver()
{
    char c;
    for (int i = 0; i < ACCOUNT_LEN; ++i)
    {
        c = sender[i];
        sender[i] = receiver[i];
        receiver[i] = c;
    }
}

int TCPMessage::createTCPMessageStream(
            char *data_buf,
            MessageType msg_typ,
            int msg_opt,
            const string& receiver,
            const string& sender,
            const vector<string>& strs
            )
{
    if (receiver.length() != 11 || sender.length() != 11)
    {
        throw 1;
        return -1;
    }
    string tmp;
    tmp = to_string(GetNowTimestamp()) + ' ';
    tmp += to_string(msg_typ) + ' ' + to_string(msg_opt) + ' ';
    char *ptr = data_buf;
    for (char c: tmp)
    {
        *ptr = c;
        ++ptr;
    }
    strcpy(ptr, sender.data());
    ptr += 12;
    *ptr = ' ';
    ++ptr;
    strcpy(ptr, receiver.data());
    ptr += 12;
    *ptr = ' ';
    ++ptr;
    int size = 0;
    for (const std::string& str: strs)
    {
        size += str.length() + 1;
    }
    tmp = to_string(size) + ' ';
    for (char c: tmp)
    {
        *ptr = c;
        ++ptr;
    }
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
    *(ptr - 1) = '\0';
    return ptr - data_buf;
}

}