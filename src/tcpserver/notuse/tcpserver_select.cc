#include "tcpserver_select.h"
#include <iostream>
#include <string.h>
#include <errno.h>


TCPServerSelect::TCPServerSelect(): data_buf(new char[MAX_BUFF_SIZE]), running(false) {}
TCPServerSelect::~TCPServerSelect() {}

BaseTCPServer* TCPServerSelect::getInstance()
{
    if (tcp_server == nullptr)
        tcp_server = new TCPServerSelect;
    return tcp_server;
}

void TCPServerSelect::beginWork()
{
    this->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket fail");
        return;
    }

    sockaddr_in ser_addr;
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(BaseTCPServer::port);
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    socklen_t socklen = sizeof(sockaddr_in);
    if (bind(server_fd, (struct sockaddr*)&ser_addr, socklen) == -1) {
        perror("bind fail");
        return;
    }

    if (listen(server_fd, BaseTCPServer::queue_size) == -1) {
        perror("fail to listen");
        return;
    }
    cout << "begin accepting" << endl;
    beginConnect();

}

void TCPServerSelect::beginConnect()
{
    fd_set rset;
    int maxfd = server_fd;
    FD_ZERO(&rset);
    //设置检测server_fd
    FD_SET(server_fd, &rset);

    timeval timevalue;
    timevalue.tv_sec = 0;
    timevalue.tv_usec = 250000;
    
    sockaddr_in client_addr;
    socklen_t socklen = sizeof(sockaddr_in);

    running = true;
    while (running)
    {
        //每次select后都会将值清零，故需重新设置
        timevalue.tv_usec = 250000;

        FD_ZERO(&rset);
        FD_SET(server_fd, &rset);
        for (ClientSocket cfd: client_fds)
        {
            FD_SET(cfd, &rset);
        }
        int nread = select(maxfd + 1, &rset, NULL, NULL, &timevalue);
        //如果错误
        if (nread == -1)
        {
            //select错误原因为按下ctrl+c结束，不打印错误信息
            if (errno == EINTR)
                continue;

            perror("select error");
            break;
        }
        //如果没有新的fd可read
        if (nread == 0)
            continue;
        //表示有新的连接请求
        if (FD_ISSET(server_fd, &rset))
        {
            cout << "a client connected." << endl;
            //因为确定有新的请求到来，所以accept不阻塞
            ClientSocket cli_fd = accept(server_fd, (sockaddr*)&client_addr, &socklen);
            client_fds.push_back(cli_fd);
            if (cli_fd > maxfd)
                ++maxfd;
            //如果是发送接收消息相关的，等待下一轮循环处理
            continue;
        }
        //有消息相关发送过来
        for (int i = 0, size = client_fds.size(); i < size; ++i)
        {
            ClientSocket& cli_fd = client_fds[i];
            if (FD_ISSET(cli_fd, &rset))
            {
                memset(data_buf, 0, MAX_BUFF_SIZE);
                int ret = recv(cli_fd, data_buf, MAX_BUFF_SIZE - 1, 0);
                //正常消息
                if (ret > 0)
                {
                    dealWithData();
                    continue;
                }
                //tcp连接断开或消息异常的处理
                close(cli_fd);
                //相当于移除该fd
                --size;
                --i;
                swap(cli_fd, client_fds[size]);
                client_fds.pop_back();
            }
        }
    }
    closeAllTCPSockets();
}

void TCPServerSelect::dealWithData()
{
    cout << data_buf << endl;
}

void TCPServerSelect::closeAllTCPSockets()
{
    for (ClientSocket cli_fd: client_fds)
    {
        close(cli_fd);
    }
    close(server_fd);
}

void TCPServerSelect::stopServer()
{
    running = false;
}