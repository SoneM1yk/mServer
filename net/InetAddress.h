#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>

class InetAddress
{
public:
    InetAddress(int16_t port)
    {
        addr_.sin_family = AF_INET;
        addr_.sin_addr.s_addr = htonl(INADDR_ANY);
        addr_.sin_port = htons(port);
        port_ = port;
    }

    InetAddress() = default;    
    
    InetAddress(sockaddr_in Addr) 
    {
        addr_ = Addr;
        int p = Addr.sin_port;
        port_ = ntohs(p);
    }

    InetAddress(const InetAddress& Addr1)
    {
        this->port_ = Addr1.port_;
        this->addr_ = Addr1.addr_;
    }
    
    static sockaddr_in getloaclAddr(int socket) {
        sockaddr_in localAddr;
        socklen_t socklen = sizeof(localAddr);
        memset(&localAddr, 0, sizeof(localAddr));
        getsockname(socket, (sockaddr*)&localAddr, &socklen);
        return localAddr;
    }

    sockaddr* getAddr()
    { return (sockaddr*)&addr_; }

    void setAddr(std::string s) 
    {
        inet_aton(s.c_str(), &addr_.sin_addr);
    }
private:
    sockaddr_in addr_;
    int16_t port_;
};