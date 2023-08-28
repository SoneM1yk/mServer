#pragma once
#include <thread>
#include <cassert>
#include <boost/utility.hpp>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <chrono>
#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <sys/eventfd.h>

#include "Timer/TimerQueue.h"
#include "Channel.h"

class Poller;
class Channel;

class EventLoop: boost::noncopyable
{
using Functor = std::function<void()>; 
public:
    EventLoop();
    ~EventLoop();

    void loop();

    void assertInLoopThread()
    {
       assert(isInLoopThread());
    }

    bool isInLoopThread() const { return threadId_ == std::this_thread::get_id(); } 
    
    EventLoop* getEventLoopOfCurrentThread();
    
    void updateChannel(Channel* channel);

    void quit() { quit_ = true; if (!isInLoopThread()) wakeup(); }
    
    void removeChannel(Channel* channel);


    void runInLoop(const Functor& cb);
    void queueInLoop(const Functor& cb);
    
    void runAt(const TimeStamp& time, const TimerCallBack& cb)
    { timerQueue_->addTimer(cb, time); }
    
    void runAfter(const TimeInterval& delay, const TimerCallBack& cb)
    { 
        TimeStamp time(steady_clock::now() + delay);
        runAt(time, cb);
    }

private:
    using ChannelList = std::vector<Channel*>;

    void handleRead();
    void doPendingFunctors();
    void wakeup();

    bool looping_;
    bool quit_;
    const std::thread::id threadId_;
    std::unique_ptr<Poller> poller_;

    TimeStamp pollReturnTime_;
    std::unique_ptr<TimerQueue> timerQueue_;

    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;
    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;
    bool callingPendingFunctors_;

    ChannelList activeChannels;
};

