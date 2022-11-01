#ifndef TCP_STANDARD_MESSAGE_H
#define TCP_STANDARD_MESSAGE_H

#include <string>

namespace tcp_standard_message
{

enum message_type
{
    invalid,
    setting,
    friends,
    groups,
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

struct TCPMessage
{
    message_type msg_typ;
    //如果msg_opt值<0,对客户端，表示请求失败；
    int msg_opt;
    //这里的长度应包括字符'\0'
    uint32_t data_len;
    //该buff无论如何应有'\0'作为结束
    char *data_buf;
    TCPMessage();
    ~TCPMessage();
    TCPMessage(TCPMessage&& msg_stru);
    TCPMessage(const TCPMessage&) = delete;
    TCPMessage& operator=(const TCPMessage& msg_stru) = delete;
    //void operator=(const std::string& str);
    void copyDataFromString(const std::string& str);
};



} // namespace tcp_standard_message
#endif