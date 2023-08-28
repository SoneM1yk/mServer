#pragma once
#include <sys/socket.h>
#include <functional>
#include "Socket.h"
#include "../Channel.h"

class EventLoop;
class InetAddress;

class Acceptor
{
public:
    using NewConnectionCallback = std::function<void(int, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& listenAddress);
    ~Acceptor() { }

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { newConnectionCallback_ = cb; }

    bool isListening() { return listening_; }
    void listen();
private:
    void handleRead();

    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
};