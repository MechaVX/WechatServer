#ifndef MYSQL_CONFIG_H
#define MYSQL_CONFIG_H

#include <vector>
#include <string>

namespace mysql_base_config
{

extern const uint16_t default_port;
extern const char *host;
extern const char *user;
extern const char *passwd;
extern const char *databasename;

namespace tables_name
{
extern const char *base_info;
extern const char *onlined;
}


}


#define USE_MYSQL_MESSAGE 0

#if USE_MYSQL_MESSAGE

namespace mysql_message
{
class MysqlReturnMessage
{
private:
    bool vaild;
    //长度包含结尾的'\0'
    int len;
    char *result;
public:
    MysqlReturnMessage();
    MysqlReturnMessage(const MysqlReturnMessage& mysql_msg);
    MysqlReturnMessage(MysqlReturnMessage&& mysql_msg);
    ~MysqlReturnMessage();
    //
    void setData(const char *data_buf);
    void setData(std::vector<std::string>& datas);
};
}
#endif

#endif