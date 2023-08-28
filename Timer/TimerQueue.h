#pragma once 
#include "Timer.h"

#include <memory>
#include <vector>
#include <map>
#include <sys/timerfd.h>
class EventLoop;
class Channel;

class TimerQueue
{
public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    void addTimer(const TimerCallBack& cb, 
        TimeStamp when);
private:
    using TimerList = std::multimap<TimeStamp, std::unique_ptr<Timer>>;

    //called when timerfd alarms
    void handleRead();

    void addInLoop(std::unique_ptr<Timer> timer);
    
    // remove expired timers
    std::vector<std::unique_ptr<Timer>> getExpired(TimeStamp now);
    
    void resetTimerfd(int timerfd_, TimeStamp when);
    bool insert(std::unique_ptr<Timer> timer);

    EventLoop* loop_;
    const int timerfd_;
    std::shared_ptr<Channel> timerfdChannel_;
    TimerList timers_;
};