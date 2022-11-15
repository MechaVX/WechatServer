#ifndef TCP_SERVER_SELECT
#define TCP_SERVER_SELECT


#include <vector>
#include "basetcpserver.h"
using namespace std;

typedef int ServerSocket;
typedef int ClientSocket;

#define MAX_BUFF_SIZE 1024

class TCPServerSelect: public BaseTCPServer
{
private:
    
    //用于储存已建立连接的fd
    vector<ClientSocket> client_fds;
    //用于存储读取的数据
    char *data_buf;
    //server是否在运行
    bool running;
    TCPServerSelect();
    void beginConnect();
    void dealWithData();
    void closeAllTCPSockets();
public:
    ~TCPServerSelect();
    static BaseTCPServer* getInstance();
    void beginWork();
    void stopServer();
};

#endif