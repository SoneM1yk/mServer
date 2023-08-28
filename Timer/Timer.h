#include <chrono>
#include <functional>

using namespace std::chrono;
using TimeStamp = steady_clock::time_point;
using TimeInterval = std::chrono::seconds ;
using TimerCallBack = std::function<void()>;


class Timer
{
public:
    Timer(const TimerCallBack& cb, TimeStamp expiration):
            expiration_(expiration),
            callBack_(cb) { };

    Timer(Timer&& timer): expiration_(timer.expiration_), callBack_(timer.callBack_),
        index_(timer.index_+1) {};
    uint32_t sequence() { return index_; }
    TimeStamp expiration() { return expiration_; }
    void run() { if (callBack_) callBack_(); } 

private:
    steady_clock::time_point expiration_;

    std::function<void()> callBack_;

    uint32_t index_;
};