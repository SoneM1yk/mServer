#include "TcpConnection.h"
#include "../EventLoop.h"


#include <sys/socket.h>
#include <unistd.h>

void TcpConnection::handleRead()
{
    size_t n = Buf_.readFd(peerSocket_->fd());
    if (n > 0) 
        messageCallback_(shared_from_this(), &Buf_, steady_clock::now());
    else if (n == 0) 
        handleClose();
}

void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    channel_->disableAll();
    closeCallback_(shared_from_this());
}

void TcpConnection::send(const std::string& message)
{
    if (state_ = kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message);
        } else {
            loop_->runInLoop([&message, this] { this->sendInLoop(message); });
        }
    }
}

void TcpConnection::sendInLoop(const std::string& message) 
{
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    if (!channel_->isWriting() && Buf_.readableBytes() == 0) {
        nwrote = ::write(peerSocket_->fd(), message.data(), message.size());
    }
    if (static_cast<size_t>(nwrote) < message.size()) {
        Buf_.append(message.data() + nwrote, message.size() - nwrote);
        if (!channel_->isWriting()) 
            channel_->enableWriting();
    }
}

void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if (channel_->isWriting()) {
        ssize_t n = ::write(peerSocket_->fd(), 
                        Buf_.peek(), Buf_.readableBytes());
        if (n > 0) {
            Buf_.retrieve(n);
            if (Buf_.readableBytes() == 0) {
                channel_->disableWriting();
                if (state_ = kDisconnecting) {
                    shutdownInLoop();
                }
            }
        }
    }
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    setState(kDisconnected);
    connectionCallback_(shared_from_this());
    loop_->removeChannel(channel_.get());
}

void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->setReadCallback([this] { this->handleRead(); });
    channel_->setWriteCallback([this] { this->handleWrite(); });
    channel_->setCloseCallback([this] { this->handleClose(); });
    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::shutdown() 
{
    if (state_ = kConnected) {
        state_ = kDisconnecting;
        loop_->queueInLoop([this] () 
            { this->shutdownInLoop(); });
    }
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if (!channel_->isWriting()) {
        ::shutdown(peerSocket_->fd(), SHUT_WR);
    }
}