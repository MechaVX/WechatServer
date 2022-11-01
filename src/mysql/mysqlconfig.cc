#include "mysqlconfig.h"
#include <string.h>

namespace mysql_base_config
{
const uint16_t default_port = 3306;
const char *host = "localhost";
const char *user = "root";
const char *passwd = "123456";
const char *databasename = "Wechat";

namespace tables_name
{
const char *base_info = "UserBaseInformation";
const char *onlined = "OnlinedUsers";
}

}

#if USE_MYSQL_MESSAGE
namespace mysql_message
{

MysqlReturnMessage::MysqlReturnMessage(): vaild(true), len(0), result(nullptr) {}

MysqlReturnMessage::MysqlReturnMessage(const MysqlReturnMessage& mysql_msg)
{
    this->len = mysql_msg.len;
    this->vaild = mysql_msg.vaild;
    this->result = new char(mysql_msg.len);
    for (int i = 0; i < mysql_msg.len; ++i)
    {
        this->result[i] = mysql_msg.result[i];
    }
}

MysqlReturnMessage::MysqlReturnMessage(MysqlReturnMessage&& mysql_msg)
{
    this->len = mysql_msg.len;
    this->vaild = mysql_msg.vaild;
    this->result = mysql_msg.result;
    mysql_msg.result = nullptr;
    mysql_msg.vaild = false;
    mysql_msg.len = 0;
}

MysqlReturnMessage::~MysqlReturnMessage()
{
    if (result != nullptr)
        delete result;
}


void MysqlReturnMessage::setData(const char *data_buf)
{
    if (result != nullptr)
        delete result;
    if (data_buf == nullptr)
    {
        this->len = 0;
        this->result = nullptr;
        this->vaild = false;
        return;
    }
    this->len = strlen(data_buf) + 1;
    this->vaild = true;
    result = new char(len);
    for (int i = 0; i < len; ++i)
    {
        result[i] = data_buf[i];
    }
}

void MysqlReturnMessage::setData(std::vector<std::string>& datas)
{
    
}

}
#endif

