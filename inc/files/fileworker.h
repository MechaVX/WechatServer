#ifndef FILE_WORKER_H
#define FILE_WORKER_H

#include <fstream>
#include <list>
#include "tcpstandardmessage.h"

using namespace std;
using tcp_standard_message::TCPMessage;

class FileWorker
{
    static const char *base_path;
    static const char *users_path;
    static const char *groups_path;
private:
    fstream worker;
    bool createDirectiors(const char *path);
public:
    void createNewAccountDir(const string& account);
    void storeTCPMessageToFile(const string& account, TCPMessage *msg_stru);
    void readTCPMessageFromFile(const string& account, list<TCPMessage>& msg_list);
};

#endif