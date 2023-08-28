#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include "InetAddress.h"

class Socket
{
public:
    Socket(int socket) { socket_ = socket; }
    ~Socket() { close(socket_); }

    Socket(Socket&) = delete;
    static int createNonblockingOrDie();


    void bindAddress(InetAddress);
    void listen();
    int accept(InetAddress&);

    int fd() { return socket_; }
private:
    int socket_;
    InetAddress localAddr_;
};