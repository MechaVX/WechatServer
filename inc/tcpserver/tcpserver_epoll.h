#ifndef TCP_SERVER_EPOLL
#define TCP_SERVER_EPOLL


#include <sys/epoll.h>
#include <unordered_set>
#include <list>
#include <mutex>
#include "basetcpserver.h"
#include "settingmessageworker.h"

using namespace std;

class TCPServerHelper;

class TCPServerEpoll: public BaseTCPServer
{
private:
    
    
    static const uint32_t max_epoll_size;
    static const uint32_t max_buf_size;
    //主要用于程序退出时关闭socket
    unordered_set<ClientSocket> client_fds;
    char *data_buf;
    TCPServerHelper *server_helper;
    //因不能立刻将接收的数据转移给server_helper而暂时存储在此
    list<pair<ClientSocket, vector<char> > > tmp_data_list;
    bool running;

    TCPServerEpoll();
    bool beginEpoll();
    //sock=0表示关闭包括server_fd在内的所有socket
    void closeSocket(int sock = 0);
    //优先将数据转移给server_helper，否则暂存至tmp_data_list
    void storeSocketReceiveData(ClientSocket client_addr, int buf_len);
public:
    static BaseTCPServer* getInstance();
    void beginWork();
    void stopServer();
    ~TCPServerEpoll();
};

#endif