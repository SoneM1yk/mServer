#include "Acceptor.h"
#include <functional>

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(loop),
    acceptSocket_(Socket::createNonblockingOrDie()),
    acceptChannel_(loop, acceptSocket_.fd()),
    listening_(false)
{
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback([this] { this->handleRead();});
}

void Acceptor::listen()
{
    loop_->assertInLoopThread();
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    loop_->assertInLoopThread();
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(peerAddr);
    if (connfd >= 0) {
        if (newConnectionCallback_)
            newConnectionCallback_(connfd, peerAddr);
    } else {
        close(connfd);
    }

}