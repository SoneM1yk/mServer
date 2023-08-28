#pragma once

#include <map>
#include <string>
#include <memory>

#include "Acceptor.h"
#include "TcpConnection.h"
#include "../EventLoopThreadPool.h"

class EventLoop;
class InetAddress;

class TcpServer
{
public:
    TcpServer(EventLoop* loop, const InetAddress& listenAddr, std::string name, int numThreads);
    ~TcpServer() {};

    //server start operating
    void start();

    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }
private:
    //called by Acceptor when new connection
    void newConnection(int sockfd, const InetAddress& peerAddr);

    //called by TcpConnection in handleClose when destroyed
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    std::unique_ptr<Acceptor> acceptor_;
    EventLoop* loop_;
    const std::string name_;

    // passed to TcpConnection to call 
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    bool started;
    int numThreads_;
    int nextConnId_;

    std::unique_ptr<EventLoopThreadPool> threadPool_;
    //stores connections
    ConnectionMap connections_;
};