#include "basemessageworker.h"

#include <iostream>

vector<string> BaseMessageWorker::splitDataBySpace(const char *data_buf)
{
    vector<string> result;
    if (data_buf == nullptr)
        return result;
    int beg_index = 0;
    int end_index = 0;
    string str(data_buf);
    
    int length = str.length();
    
    while (end_index < length)
    {
        while (str[end_index] != ' ' && end_index < length)
            ++end_index;
        result.push_back(str.substr(beg_index, end_index - beg_index));
        ++end_index;
        beg_index = end_index;
    }
    return std::move(result);
}

vector<string> BaseMessageWorker::splitDataBySpace(const char *data_buf, int len)
{

}