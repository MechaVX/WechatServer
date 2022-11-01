#define USE_COMMON 0
#define USE_SELECT 1

#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#include "tcpserver.h"

BaseTCPServer *tcp_server;
void ctrl_c_handler(int sig)
{
    if (tcp_server == nullptr)
        return;
    tcp_server->stopServer();
    exit(0);
}

int main(int argc, char const *argv[])
{
    signal(SIGINT, ctrl_c_handler);
    tcp_server = TCPServer::getInstance();
    tcp_server->beginWork();
    return 0;
}


