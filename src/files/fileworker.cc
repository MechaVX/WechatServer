#include "fileworker.h"

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

const char* FileWorker::base_path = "/home/scrin/Desktop/linux_program/net_work/Wechat/cache/";
const char* FileWorker::users_path = "/home/scrin/Desktop/linux_program/net_work/Wechat/cache/users/";
const char* FileWorker::groups_path = "/home/scrin/Desktop/linux_program/net_work/Wechat/cache/groups/";

bool FileWorker::createDirectiors(const char *path)
{
    char str[512];
    strcpy(str, path);
    int len = strlen(str);
    for (int i = 0; i < len; i++)
    {
        if (str[i] == '/' )
        {
            str[i] = '\0';
            if (access(str, 0) != 0 )
            {
                mkdir(str, 0755);
            }
            str[i] = '/';
        }
    }
    if (len > 0 && access(str, 0) != 0 )
    {
        mkdir(str, 0766);
    }
	struct stat s;
	stat(path, &s);
	if (S_ISDIR(s.st_mode))
        return false;
    return true;
}

void FileWorker::createNewAccountDir(const string& account)
{
    string path = users_path + account + '/';
    cout << "FileWorker::createNewAccountDir:\n" << path << endl;
    createDirectiors(path.data());
}

void FileWorker::storeTCPMessageToFile(const string& account, TCPMessage *msg_stru)
{
    string file = users_path + account + "/message.msg";
    worker.open(file, ios::app);
    uint32_t size = sizeof(TCPMessage);
    worker.write((char*)msg_stru, size);
    worker.write(msg_stru->data_buf, msg_stru->data_len);
    worker.close();
}

//参考http://c.biancheng.net/view/309.html
void FileWorker::readTCPMessageFromFile(const string& account, list<TCPMessage>& msg_list)
{
    string file = users_path + account + "/message.msg";
    int size = sizeof(TCPMessage);
    worker.open(file, ios::in | ios::binary);
    while (worker.peek() != EOF)
    {
        msg_list.push_back(TCPMessage());
        TCPMessage& msg_stru = msg_list.back();
        worker.read((char*)&msg_stru, size);
        int buf_len = msg_stru.data_len;
        msg_stru.data_buf = new char[buf_len];
        worker.read(msg_stru.data_buf, buf_len);
    }
    worker.close();
    //好像还没找到其他办法清空文件
    worker.open(file, ios::out);
    worker.close();
    if (!msg_list.empty())
    {
        cout << "readTCPMessageFromFile" << endl;
        for (auto& msg: msg_list)
            cout << msg << endl;
    }
    
}