#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#include "tcpserver_select.h"

BaseTCPServer *tcp_server;
void ctrl_c_handler(int sig)
{
    if (tcp_server == nullptr)
        return;
    tcp_server->stopServer();
    cout << "bye" << endl;
}

int main(int argc, char const *argv[])
{
    signal(SIGINT, ctrl_c_handler);
    tcp_server = TCPServerSelect::getInstance();
    tcp_server->beginWork();
    return 0;
}