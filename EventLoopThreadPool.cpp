#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop)
    : baseLoop_(baseLoop),
    started_(false),
    numThreads_(0),
    next_(0)
{ }

EventLoopThreadPool::~EventLoopThreadPool()
{ 

}

void EventLoopThreadPool::start()
{
    baseLoop_->assertInLoopThread();

    started_ = true;
    for (int i=0; i<numThreads_; i++) {
        threads_.emplace_back(std::make_unique<EventLoopThread>());
        loops_.push_back(threads_.back()->startLoop());
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    baseLoop_->assertInLoopThread();
    assert(started_);

    EventLoop* loop = baseLoop_;
    if (!loops_.empty()) {
        loop = loops_[(next_++)%numThreads_];
    }
    return loop;
}