#include "TcpServer.h"
#include "InetAddress.h"

TcpServer::TcpServer(EventLoop* loop, 
    const InetAddress& listenAddr, std::string name, int numThreads):
    loop_(loop), 
    acceptor_(std::make_unique<Acceptor>(loop, listenAddr)), 
    name_(name),
    numThreads_(numThreads),
    threadPool_(std::make_unique<EventLoopThreadPool>(loop))
    { 
        acceptor_->setNewConnectionCallback(
            [this] (const int sockfd, const InetAddress& peerAddr) {
                newConnection(sockfd, peerAddr);
            });
        threadPool_->setThreadNum(numThreads);
    }

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof(buf), "#%d", nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    InetAddress localAddr(InetAddress::getloaclAddr(sockfd));
    EventLoop* ioLoop = threadPool_->getNextLoop();
    TcpConnectionPtr conn(std::make_shared<TcpConnection>(ioLoop, connName,
         sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(
        [this] (TcpConnectionPtr connect) {
            this->removeConnection(connect);
        });
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::start()
{
    threadPool_->start();    
    acceptor_->listen();
    loop_->loop();
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) 
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) 
{
    int n = connections_.erase(conn->name());
    EventLoop* ioLoop = conn->getLoop();
    assert(n==1);
    ioLoop->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}