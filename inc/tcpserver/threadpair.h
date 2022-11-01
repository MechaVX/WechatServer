#ifndef THREAD_PAIR_H
#define THREAD_PAIR_H

#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

class ThreadPair
{
private:
    
    mutex send_buf_mutex;
    condition_variable condition;
    
    bool client_close;
    //线程是否已经完成的标志
    bool threads_not_completed;
public:
    static const uint16_t max_buf_size;

    string ip_port;
    thread *send_thread;
    thread *receive_thread;
    const char *send_buf;
    char *receive_buf;

    ThreadPair();
    ~ThreadPair();
    ThreadPair(const ThreadPair&) = delete;
    ThreadPair& operator=(const ThreadPair&) = delete;

    void sendData(const char *data_buf);
    //如果client关闭，返回false
    bool blockToWaitingData();
    void notifyThreadsToExit();
    //由接收线程调用，等待发送线程执行完毕并修改结束标志
    void waitSendThreadToExit();
    //判断线程是否已经结束，表示客户端是否已经关闭
    bool isThreadCompleted();
    //提供等待线程结束的方法
    void joinThread();
};

#endif