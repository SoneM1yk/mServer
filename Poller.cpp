#include "Poller.h"
#include "Channel.h"
#include <cstdio>

std::chrono::steady_clock::time_point
    Poller::poll(int timeoutMs, ChannelList* activeChannels) 
{
    int numEvents = ::poll(pollfds_.data(), pollfds_.size(), 0);

    auto now = std::chrono::steady_clock::now();
    if (numEvents>0) {
        fillActiveChannels(numEvents, activeChannels);
    } else if (numEvents == 0) {
        
    } else {

    }

    return now;
}

void Poller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    for (auto pfd = pollfds_.cbegin(); pfd!=pollfds_.cend() && numEvents>0; ++pfd) {
        if (pfd->revents > 0) {
            --numEvents;
            ChannelMap::const_iterator ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            Channel* channel = ch->second;
            channel->set_revents(pfd->revents);
            activeChannels->push_back(channel);
        }
    }
}

void Poller::updateChannel(Channel* channel) 
{
    assertInLoopThread();
    if (channel->index() < 0) {
        //create corresponding pollfd
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        //insert map
        int idx = static_cast<int>(pollfds_.size())-1;
        channel->set_index(idx);
        channels_[pfd.fd] = channel;
    } else {
        //modify the existent pollfd
        int idx = channel->index();
        pollfd& pfd = pollfds_[idx];
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
    }
}

void Poller::removeChannel(Channel* channel) 
{
    int idx = channel->index();
    Channel* endChannel = channels_[pollfds_.back().fd];
    endChannel->set_index(idx);
    if (idx != pollfds_.size()-1) {
        std::swap(pollfds_[idx], pollfds_.back());
    }
    pollfds_.pop_back();
    channels_.erase(channel->fd());
}