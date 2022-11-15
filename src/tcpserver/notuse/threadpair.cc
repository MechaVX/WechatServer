#include "threadpair.h"


const uint16_t ThreadPair::max_buf_size = 1024;

ThreadPair::ThreadPair(): send_thread(nullptr), receive_thread(nullptr)
{
    threads_not_completed = true;
    client_close = false;
    send_buf = nullptr;
    receive_buf = new char[ThreadPair::max_buf_size];
}

ThreadPair::~ThreadPair()
{
    this->joinThread();
    if (send_thread != nullptr)
    {
        delete send_thread;
    }
    if (receive_thread != nullptr)
    {
        delete receive_thread;
    }
    delete receive_buf;
}

bool ThreadPair::blockToWaitingData()
{
    unique_lock<mutex> lock(send_buf_mutex);
    send_buf = nullptr;
    condition.wait(lock, [this]()
    {
        return send_buf != nullptr || client_close;
    });
    return !client_close;
}

void ThreadPair::notifyThreadsToExit()
{
    client_close = true;
    unique_lock<mutex> lock(send_buf_mutex);
    condition.notify_one();
}

void ThreadPair::waitSendThreadToExit()
{
    send_thread->join();
    threads_not_completed = false;
}

bool ThreadPair::isThreadCompleted()
{
    return !threads_not_completed;
}

void ThreadPair::joinThread()
{
    if (send_thread != nullptr && send_thread->joinable())
    {
        send_thread->join();
    }
    if (receive_thread != nullptr && receive_thread->joinable())
    {
        receive_thread->join();
    }
}