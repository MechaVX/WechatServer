#include "tcpstandardmessage.h"

#include <string.h>

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
    strcpy(data_buf, str.data());
}

}