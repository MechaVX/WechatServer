#include "tcpserver.h"
#include "threadpair.h"

#include <string.h>
#include <iostream>
#include <list>

TCPServer::TCPServer(): server_run(false) {}

TCPServer::~TCPServer() {}

BaseTCPServer* TCPServer::getInstance()
{
    if (tcp_server == nullptr)
        tcp_server = new TCPServer;
    return tcp_server;
}

void TCPServer::beginWork()
{
    this->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket fail");
        return;
    }

    sockaddr_in ser_addr;
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(TCPServer::port);
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    socklen_t socklen = sizeof(sockaddr_in);
    if (bind(server_fd, (struct sockaddr*)&ser_addr, socklen) == -1) {
        perror("bind fail");
        return;
    }

    if (listen(server_fd, TCPServer::queue_size) == -1) {
        perror("fail to listen");
        return;
    }

    this->serverKeepAccepting(socklen);

}

void TCPServer::serverKeepAccepting(socklen_t socklen)
{
    cout << "begin accepting" << endl;
    sockaddr_in client_addr;
    while (1)
    {
        //accept阻塞
        ClientSocket cli_soc = accept(server_fd, (sockaddr*)&client_addr, &socklen);
        if (cli_soc == -1)
        {
            perror("fail to accept");
            break;
        }
        //首先检查一次是否有socket已经关闭，并将其从map中移除
        removeCompletedClientSocket();
        dealWithClientSocket(client_addr, cli_soc);
    }
}

void TCPServer::dealWithClientSocket(sockaddr_in client_addr, ClientSocket cli_soc)
{
    //获取ip和端口号
    char cli_ip[16]{};
    inet_ntop(AF_INET, &client_addr.sin_addr, cli_ip, 16);
    uint16_t cli_port = ntohs(client_addr.sin_port);
    string ip_port = string(cli_ip) + ':' +to_string(cli_port);
    cout << ip_port << " connected" << endl;
    {
        lock_guard<mutex> lock(ipport_socket_map_mutex);
        ipport_socket_map[ip_port] = cli_soc;
    }
    
    {
        shared_ptr<ThreadPair> thr_pair = make_shared<ThreadPair>();
        thr_pair->ip_port = std::move(ip_port);
        lock_guard<mutex> lock(clisoc_thrpair_map_mutex);
        clisoc_thrpair_map[cli_soc] = thr_pair;
        thr_pair->send_thread = new thread(TCPServer::sendDataToClient, thr_pair, cli_soc);
        thr_pair->receive_thread = new thread(TCPServer::receiveDataFromClient, thr_pair, cli_soc);
    }
    
}

void TCPServer::sendDataToClient(shared_ptr<ThreadPair> thr_pair, ClientSocket cli_soc)
{
    while (1)
    {
        if (!thr_pair->blockToWaitingData())
            break;
        if (send(cli_soc, thr_pair->send_buf, strlen(thr_pair->send_buf), 0))
            break;
        thr_pair->send_buf = nullptr;
    }
}

void TCPServer::receiveDataFromClient(shared_ptr<ThreadPair> thr_pair, ClientSocket cli_soc)
{
    while (1)
    {
        memset(thr_pair->receive_buf, 0, ThreadPair::max_buf_size);
        int flag = recv(cli_soc, thr_pair->receive_buf, ThreadPair::max_buf_size, 0);
        if (flag == 0)
        {
            thr_pair->notifyThreadsToExit();
            break;
        }
        if (thr_pair->receive_buf)
            cout << thr_pair->receive_buf << endl;
    }
    thr_pair->waitSendThreadToExit();
}

void TCPServer::removeCompletedClientSocket()
{
    //要移除已关闭的socket
    list<ClientSocket> sockets;
    //要移除已关闭的ip和port
    list<string> ip_ports;

    {
        lock_guard<mutex> lock(clisoc_thrpair_map_mutex);
        for (auto it = clisoc_thrpair_map.begin(); it != clisoc_thrpair_map.end(); ++it)
        {
            if (it->second->isThreadCompleted())
            {
                ip_ports.push_back(std::move(it->second->ip_port));
                sockets.push_back(it->first);
            }
        }
        for (ClientSocket cli_soc: sockets)
        {
            clisoc_thrpair_map.erase(clisoc_thrpair_map.find(cli_soc));
        }
    }

    {
        lock_guard<mutex> lock(ipport_socket_map_mutex);
        for (string& ip_port: ip_ports)
        {
            ipport_socket_map.erase(ipport_socket_map.find(ip_port));
        }
    }

}

void TCPServer::stopServer()
{
    {
        lock_guard<mutex> lock(ipport_socket_map_mutex);
        for (auto& pair_: ipport_socket_map)
        {
            close(pair_.second);
        }
    }
    {
        lock_guard<mutex> lock(clisoc_thrpair_map_mutex);
        for (auto& pair_: clisoc_thrpair_map)
        {
            pair_.second->joinThread();
        }
    }
    close(server_fd);
}