#ifndef BASE_MESSAGE_WORKER_H
#define BASE_MESSAGE_WORKER_H

#include <vector>
#include <string>

using namespace std;

class BaseMessageWorker
{
protected:
    
    //该函数用于按照空格分割字符串，data_buf中间不能包含'\0'
    virtual vector<string> splitDataBySpace(const char *data_buf);
    //如果data_buf中间可能包含'\0'，使用该函数
    virtual vector<string> splitDataBySpace(const char *data_buf, int len);
};

#endif