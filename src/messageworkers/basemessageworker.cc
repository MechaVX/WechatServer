#include "basemessageworker.h"

#include <iostream>

vector<string> BaseMessageWorker::splitDataBySpace(const char *data_buf)
{
    int beg_index = 0;
    int end_index = 0;
    string str(data_buf);
    
    int length = str.length();
    vector<string> result;
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