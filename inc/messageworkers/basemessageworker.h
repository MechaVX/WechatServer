#ifndef BASE_MESSAGE_WORKER_H
#define BASE_MESSAGE_WORKER_H

#include <vector>
#include <string>
#include "globaldefine.h"
#include "fileworker.h"

using namespace std;

class BaseMessageWorker
{
protected:
    FileWorker file_worker;
    //该函数用于按照空格分割字符串，data_buf中间不能包含'\0'
    //对位于buf首尾的空格暂不能很好处理
    vector<string> splitDataBySpace(const char *data_buf);
    //如果data_buf中间可能包含'\0'，使用该函数
    vector<string> splitDataBySpace(const char *data_buf, int len);
};

#endif