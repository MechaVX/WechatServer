#include "tcpserver_epoll.h"
#include "tcpserverhelper.h"
#include "tcpstandardmessage.h"
#include <iostream>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

const uint32_t TCPServerEpoll::max_epoll_size = 30;
const uint32_t TCPServerEpoll::max_buf_size = 5120;

TCPServerEpoll::TCPServerEpoll(): data_buf(new char[max_buf_size]), running(false)
{
    server_helper = new TCPServerHelper;
}
TCPServerEpoll::~TCPServerEpoll()
{
    delete data_buf;
    delete server_helper;
}

BaseTCPServer* TCPServerEpoll::getInstance()
{
    if (tcp_server == nullptr)
        tcp_server = new TCPServerEpoll;
    return tcp_server;
}


void TCPServerEpoll::beginWork()
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
    beginEpoll();
}

bool TCPServerEpoll::beginEpoll()
{
    int efd = epoll_create(10);
    if (efd == -1)
    {
        perror("epoll fail");
        return false;
    }

    epoll_event ev, evsarr[TCPServerEpoll::max_epoll_size];
    memset(evsarr, 0, sizeof(evsarr));
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, server_fd, &ev) == -1)
    {
        perror("epoll_ctl error");
        return false;
    }

    sockaddr_in client_addr;
    socklen_t socklen = sizeof(sockaddr_in);

    if (!server_helper->serverHelperStart())
    {
        cout << "beginEpoll fail" << endl;
        return false;
    }

    running = true;
    while (running)
    {
        
        //等待时间单位是ms
        int nread = epoll_wait(efd, evsarr, TCPServerEpoll::max_epoll_size, 250);
        //尝试将上次接收但未转移的消息传给server_helper
        if (!this->tmp_data_list.empty())
        {
            bool success = server_helper->storeSocketReceiveData(tmp_data_list);
            if (success)
                tmp_data_list.clear();
        }
        //出错
        if (nread == -1)
        {
            if (errno == EINTR)
                continue;
            perror("epoll_wait fail");
            return false;
        }
        for (int i = 0; i < nread; ++i)
        {
            //有新的连接请求建立
            if (evsarr[i].data.fd == server_fd)
            {
                int cli_fd = accept(server_fd, (sockaddr*)&client_addr, &socklen);
                if (cli_fd == -1)
                    continue;
                //fd增添非阻塞属性
                int old_fd_flag = fcntl(cli_fd, F_GETFL, 0);
                int new_fd_flag = old_fd_flag | O_NONBLOCK;
                if (fcntl(cli_fd, F_SETFL, new_fd_flag) == -1)
                {
                    close(cli_fd);
                    cout << "set clientfd to nonblocking error." << endl;
                }
                else
                {
                    ev.data.fd = cli_fd;
                    if (epoll_ctl(efd, EPOLL_CTL_ADD, cli_fd, &ev) != -1)
                    {
                        cout << "new socket accept" << endl;
                        client_fds.insert(cli_fd);
                    }
                    else
                    {
                        cout << "add client fd to epollfd error" << endl;
                        closeSocket(cli_fd);
                    }
                }
                
            }
            //其他可读消息
            else
            {
                //memset(data_buf, 0, max_buf_size);
                int nbytes = recv(evsarr[i].data.fd, data_buf, max_buf_size, 0);
                if (nbytes > 0)
                {
                    this->storeSocketReceiveData(evsarr[i].data.fd, nbytes);
                }
                else if (nbytes == -1)
                {
                    cout << "clientfd receive data error, close it" << endl;
                    closeSocket(evsarr[i].data.fd);
                }
                else
                {
                    cout << "a client disconnect, close fd" << endl;
                    closeSocket(evsarr[i].data.fd);
                }
            }
        }
    }
    closeSocket();
    return true;
}

void TCPServerEpoll::storeSocketReceiveData(ClientSocket cli_soc, int buf_len)
{
    //如果转移成功，直接返回
    bool success = server_helper->storeSocketReceiveData(cli_soc, this->data_buf, buf_len);
    if (success)
        return;
    //转移不成功，暂存
    vector<char> tmp;
    tmp.reserve(buf_len);
    for (int i = 0; i < buf_len; ++i)
    {
        tmp.push_back(data_buf[i]);
    }
    tmp_data_list.emplace_back(cli_soc, std::move(tmp));
}

void TCPServerEpoll::stopServer()
{
    closeSocket();
    running = false;
}

void TCPServerEpoll::closeSocket(int sock)
{
    if (sock > 0)
    {
        close(sock);
        client_fds.erase(client_fds.find(sock));
        //考虑到用户可能异常退出，这种情况下由服务器自己产生用户退出登录的数据包
        int len = TCPMessage::createTCPMessageStream(setting, user_logout, { to_string(sock) }, data_buf);
        this->storeSocketReceiveData(sock, len);
        return;
    }
    if (sock == 0)
    {
        for (ClientSocket cli_fd: client_fds)
        {
            close(cli_fd);
        }
        close(server_fd);
    }

    
}