#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "basetcpserver.h"

#include <string>
#include <unordered_map>
#include <thread>
#include <condition_variable>
#include <memory>
#include <mutex>

using namespace std;

class ThreadPair;

class TCPServer: public BaseTCPServer
{
private:

    mutex ipport_socket_map_mutex;
    //由ip＋port找到对于socket
    unordered_map<string, ClientSocket> ipport_socket_map;

    mutex clisoc_thrpair_map_mutex;
    //由socket找到对应线程对
    unordered_map<ClientSocket, shared_ptr<ThreadPair> > clisoc_thrpair_map;

    bool server_run;

    TCPServer();
    void serverKeepAccepting(socklen_t socklen);
    //处理建立连接请求的client
    void dealWithClientSocket(sockaddr_in client_addr, ClientSocket cli_soc);

    //如果某个socket的线程退出完成，将socket移除
    void removeCompletedClientSocket();

    
public:
    ~TCPServer();
    //暂不考虑线程安全的问题
    static BaseTCPServer* getInstance();
    //调用该函数的线程将持续监听tcp端口阻塞，可按ctrl+c结束程序
    void beginWork();

    //主要用于join线程
    void stopServer();
    
    //以下两函数在两新线程中调用
    static void sendDataToClient(shared_ptr<ThreadPair> thr_pair, ClientSocket cli_soc);
    static void receiveDataFromClient(shared_ptr<ThreadPair> thr_pair, ClientSocket cli_soc);
};

#endif