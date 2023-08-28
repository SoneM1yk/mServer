#pragma once

#include "EventLoop.h"
#include <functional>

class EventLoop;

class Channel
{
public:
    using EventCallback = std::function<void()>;

    Channel(EventLoop* loop, int fd);

    void handleEvent();
    void setReadCallback(const EventCallback& cb)
    { readCallback_ = cb; }
    void setWriteCallback(const EventCallback& cb) 
    { writeCallback_ = cb; }
    void setErrorCallback(const EventCallback& cb)
    { errorCallback_ = cb; }
    void setCloseCallback(const EventCallback& cb)
    { closeCallback_ = cb; }
    void set_revents(int revents) 
    { revents_ = revents; }
    void set_index(int index) 
    { index_ = index; }
    void set_events(int events) 
    { events_ = events; }

    void enableReading() 
    { events_ |= kReadEvent; update(); }
    void enableWriting() 
    { events_ |= kWriteEvent; update(); }
    void disableWriting()
    { events_ &= ~kWriteEvent; update(); }
    void disableAll() 
    { events_ = kNoneEvent; update(); }
    bool isWriting()
    { return events_ & kWriteEvent; }
    
    int index() const { return index_; }
    int fd() const {return fd_; }
    int events() const { return events_; }
    void update();
private:

    static const int kNoneEvent, kReadEvent, kWriteEvent;
    EventCallback readCallback_, writeCallback_, errorCallback_, closeCallback_;

    EventLoop* loop_;
    const int fd_;

    //concerned events
    int events_;
    
    //catched events
    int revents_;

    //index in Poller::pollfds_
    int index_;
};