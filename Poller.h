#include "Channel.h"
#include <vector>
#include <map>

struct pollfd;

class Poller
{
public:
    typedef std::vector<Channel*> ChannelList;

    Poller(EventLoop* loop):
        ownerLoop_(loop)
    { }
    ~Poller() { }

    // called by EventLoop
    //return activeChannel list through activeChannels(should be constructed)
    std::chrono::steady_clock::time_point
        poll(int timeoutMs, ChannelList* activeChannels);
    
    //update Channel in channels_
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    void assertInLoopThread() { ownerLoop_->assertInLoopThread(); };
private:
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
    
    using PollFdList = std::vector<pollfd>;
    using ChannelMap = std::map<int, Channel*>;

    EventLoop* ownerLoop_;
    PollFdList pollfds_;
    ChannelMap channels_;
};