#include "Socket.h"
#include <cstdio>

int Socket::createNonblockingOrDie()
{
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);

    return sockfd;
}

void Socket::listen()
{
    ::listen(socket_, 5);
}

void Socket::bindAddress(InetAddress Addr)
{
    socklen_t addrlen = sizeof(sockaddr);
    localAddr_ = Addr;
    bind(socket_, Addr.getAddr(), addrlen);
}

int Socket::accept(InetAddress& addr) 
{
    socklen_t addrlen = sizeof(sockaddr_in);
    int connfd = ::accept(socket_, addr.getAddr(), &addrlen);
    if (connfd < 0) {
        printf("Error in accept");
    }
    return connfd;
}