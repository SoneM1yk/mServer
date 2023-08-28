#include "EventLoop.h"
#include "Poller.h"

thread_local EventLoop* t_loopInThisThread = nullptr;

EventLoop::EventLoop() 
    : looping_(false),
    threadId_(std::this_thread::get_id()),
    poller_(new Poller(this)),
    timerQueue_(std::make_unique<TimerQueue>(this))
{
    if (t_loopInThisThread) {
        
    } else {
        t_loopInThisThread = this;
    } 
    wakeupFd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    wakeupChannel_ = std::make_unique<Channel>(this, wakeupFd_);
    wakeupChannel_->setReadCallback([this]() { handleRead(); });
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() 
{
    t_loopInThisThread = nullptr;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

void EventLoop::loop()
{
    assertInLoopThread();
    looping_ = true;
    quit_ = false;
    while (!quit_) {
        activeChannels.clear();
        poller_->poll(30, &activeChannels);
        for (auto it=activeChannels.begin(); it!=activeChannels.end(); ++it) {
            (*it)->handleEvent();
        }
        doPendingFunctors();
    }
    looping_ = false;
}

void EventLoop::updateChannel(Channel* channel) 
{
    poller_->updateChannel(channel);
}

void EventLoop::runInLoop(const Functor& cb) 
{
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Functor& cb)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(cb);
    }
    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for (int i=0; i!=functors.size(); ++i) {
        functors[i]();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::wakeup()
{
    int one = 1;
    write(wakeupFd_, &one, sizeof(one));
}

void EventLoop::handleRead()
{
    int one = 0;
    read(wakeupFd_, &one, sizeof(one));
}

void EventLoop::removeChannel(Channel* channel)
{
    poller_->removeChannel(channel);
}