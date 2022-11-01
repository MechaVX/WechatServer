#ifndef BASE_TCP_SERVER
#define BASE_TCP_SERVER

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "globaldefine.h"

class BaseTCPServer
{
protected:
    static const uint16_t port;
    static const uint32_t queue_size;
    static BaseTCPServer *tcp_server;
    ServerSocket server_fd;
public:
    virtual void beginWork() = 0;
    virtual void stopServer() = 0;
    virtual ~BaseTCPServer();
};



#endif