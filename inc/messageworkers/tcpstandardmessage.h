#ifndef TCP_STANDARD_MESSAGE_H
#define TCP_STANDARD_MESSAGE_H

#include <string>
#include <ostream>
#include <memory>
#include <vector>

using std::string;
using std::ostream;
using std::shared_ptr;
using std::vector;

namespace tcp_standard_message
{

enum MessageType
{
    //值为invalid，表示不需要发送
    invalid,
    setting,
    friends,
    groups,
    //指客户端主动刷新消息
    flush,
};

enum UserSetting
{
    user_register,
    user_login,
    user_logout,
    change_password,
    rename_account,
    add_friends_label,
    remove_friends_label,
};

enum FriendOperation
{
    search_someone,
    add_friend,
    agree_add_friend,
    remove_friend,
    send_message,
    send_file,
    change_nickname,
    add_friend_to_group,
    invite_friend_to_group,
    recommend_friend,
};

enum GroupsOperation
{
    search_group,
    join_group,
    agree_add_group,
    exit_group,
    create_group,
    dispersed_group,
    remove_someone_from_group,
    rename_group,
    set_group_join_attribute,
    change_group_master,
    disable_group_communication,
    enable_group_communication,
};

enum FlushOperation
{
    flush_message,
    flush_friends,
    flush_group,
    flush_setting,
};

struct TCPMessage
{
    MessageType msg_typ;
    //如果msg_opt值<0,对客户端，表示请求失败；
    int msg_opt;
    //这里的长度应包括字符'\0'
    uint32_t data_len;
    //除非data_len=0时,data_buf=nullptr，该buff无论如何应有'\0'作为结束
    char *data_buf;
    TCPMessage();
    ~TCPMessage();
    TCPMessage(TCPMessage&& msg_stru);
    TCPMessage(const TCPMessage&) = delete;
    TCPMessage& operator=(const TCPMessage& msg_stru) = delete;
    void operator=(TCPMessage&& msg_stru);
    friend ostream& operator<<(ostream& out_, const TCPMessage& msg_stru);
    //自动添加末尾'\0'
    void copyDataFromString(const std::string& str);
    //返回的string一定包含结尾'\0'
    string serializeToStdString() const;
    static shared_ptr<TCPMessage> createTCPMessage(MessageType msg_typ, int msg_opt, const vector<string>& strs);
    //该方法直接产生原始的数据流，返回生成后包括'\0'字符的长度
    //要保证data_buf足够长
    static int createTCPMessageStream(MessageType msg_typ, int msg_opt, const vector<string>& strs, char *data_buf);
};



} // namespace tcp_standard_message
#endif