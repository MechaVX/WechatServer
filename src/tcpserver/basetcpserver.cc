#include "basetcpserver.h"

const uint16_t BaseTCPServer::port = 7777;
const uint32_t BaseTCPServer::queue_size = 15;
BaseTCPServer* BaseTCPServer::tcp_server = nullptr;

BaseTCPServer::~BaseTCPServer() {}