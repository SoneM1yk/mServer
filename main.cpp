#include "net/TcpServer.h"
#include "EventLoop.h"
#include "net/InetAddress.h"
#include <stdio.h>
#include <iostream>

void onConnection(const TcpConnectionPtr& conn)
{
  std::cout<<"Connection"<<std::endl;
}

void onMessage(const TcpConnectionPtr& conn,
               Buffer* buf,
               TimeStamp receiveTime)
{
  std::cout<<buf->retrieveAllAsString()<<std::endl;
}

int main()
{
  printf("main(): pid = %d\n", getpid());

  InetAddress listenAddr(9981);
  EventLoop loop;

  TcpServer server(&loop, listenAddr, "Server",3);
  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);
  server.start();

  loop.loop();
}