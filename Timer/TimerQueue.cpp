#include "TimerQueue.h"
#include "../EventLoop.h"
#include <algorithm>

TimerQueue::~TimerQueue()
{ 

}

std::vector<std::unique_ptr<Timer>> TimerQueue::getExpired(TimeStamp now)
{
    std::vector<std::unique_ptr<Timer>> expired;
    TimerList::iterator it = timers_.lower_bound(now);
    assert(it==timers_.end() || now < it->first);
    for (auto iter = timers_.begin(); iter!=it; iter++) {
        expired.push_back(std::move(iter->second));
    }
    timers_.erase(timers_.begin(), it);

    return expired;

}

void TimerQueue::addTimer(const TimerCallBack& cb, TimeStamp ending)
{
    std::unique_ptr<Timer> timer_(new Timer(cb, ending));
    loop_->runInLoop(  [&timer_, this]() { 
        addInLoop(std::move(timer_));
    });
}

void TimerQueue::addInLoop(std::unique_ptr<Timer> timer_) 
{
    TimeStamp newEndingTime = timer_->expiration();
    bool EarliestChanged_ = insert(std::move(timer_));
    if (EarliestChanged_) {
        resetTimerfd(timerfd_, newEndingTime); 
    } 
}

void TimerQueue::handleRead() 
{
    auto now = std::chrono::steady_clock::now();
    ssize_t n;
    read(timerfd_, &n, sizeof(n));
    auto expired = getExpired(now);
    for (auto it = expired.begin(); it!=expired.end(); it++) {
        (*it)->run();
    }
    if (!timers_.empty())
        resetTimerfd(timerfd_, timers_.begin()->first);
}

TimerQueue::TimerQueue(EventLoop* loop) :
    loop_(loop), 
    timerfd_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC))
{
    timerfdChannel_.reset(new Channel(loop, timerfd_));
    timerfdChannel_->setReadCallback([this]() { handleRead(); });
    timerfdChannel_->enableReading();
}

void TimerQueue::resetTimerfd(int timerfd, TimeStamp newExpired) 
{   
    itimerspec t = {};
    seconds sec = duration_cast<seconds>(newExpired - std::chrono::steady_clock::now());
    uint64_t num = sec.count();
    t.it_value.tv_sec = num;
    timerfd_settime(timerfd, 0, &t, nullptr);
}

bool TimerQueue::insert(std::unique_ptr<Timer> timer)
{
    TimeStamp timerEnding = timer->expiration();
    auto it = timers_.begin();
    bool earliestChanged = false;
    if (it == timers_.end() || timerEnding < it->first)
        earliestChanged = true;
    timers_.insert({timerEnding, std::move(timer)});
    return earliestChanged; 
}