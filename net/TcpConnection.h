#pragma once
#include <memory>
#include <functional>
#include "Socket.h"
#include "InetAddress.h"
#include "../Channel.h"
#include "../Buffer.h"


class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr&, Buffer*, TimeStamp)> MessageCallback;
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;

class EventLoop;
class Channel;

class TcpConnection: public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop, std::string name, int sockfd, InetAddress localAddr, 
        InetAddress peerAddr): 
        loop_(loop), 
        name_(name), 
        peerSocket_(new Socket(sockfd)), 
        localAddr_(localAddr),
        peerAddr_(peerAddr) , 
        channel_(std::make_unique<Channel>(loop, sockfd)),
        state_(kConnecting)
        { } 
    ~TcpConnection() {  }

    EventLoop* getLoop() 
    { return loop_;}

    std::string name() { return name_; }

    void setCloseCallback(const CloseCallback& cb)
    { closeCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback& cb) 
    { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) 
    { messageCallback_ = cb; }


    void connectEstablished();

    //called by EventLoop::queueInLoop in TcpServer::RemoveConnection
    void connectDestroyed(); 

    void send(const std::string& message);
    void shutdown();

private:
    enum StateE { kConnecting, kConnected, kDisconnecting, kDisconnected};

    void sendInLoop(const std::string& message);
    void shutdownInLoop();

    void setState(StateE s) { state_ = s; };
    void handleRead();
    void handleWrite();
    void handleClose();

    Buffer Buf_;

    EventLoop* loop_;
    std::string name_;
    StateE state_;
    std::unique_ptr<Socket> peerSocket_;
    std::unique_ptr<Channel> channel_;
    InetAddress localAddr_;
    InetAddress peerAddr_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
};