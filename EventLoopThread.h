#include "EventLoop.h"
#include <thread>
#include <condition_variable>
#include <mutex>


class EventLoopThread
{
public:
    EventLoopThread():loop_(nullptr) 
    { }

    ~EventLoopThread() 
    {
        if (loop_!=nullptr) {
            loop_->quit();
            thread_->join();
        }
    }

    EventLoop* startLoop()
    {   
        thread_ = std::make_unique<std::thread>([this]() { threadFunc(); });
        std::unique_lock<std::mutex> lk(mutex_);
        while (loop_ == nullptr) {
            cond_.wait(lk, [&] { return loop_ != nullptr; });
        }
        return loop_;
    }
private:
    void threadFunc()
    {
        EventLoop loop;
        {
            std::lock_guard<std::mutex> lk(mutex_);
            loop_ = &loop;
            cond_.notify_one();
        }
        loop_->loop();

        std::lock_guard<std::mutex> lk(mutex_);
        loop_ = nullptr;
    }

    std::unique_ptr<std::thread> thread_;
    std::condition_variable cond_;
    
    std::mutex mutex_;
    EventLoop* loop_;

};  