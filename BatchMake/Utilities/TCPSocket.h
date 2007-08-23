#ifndef TCPSocket_h
#define TCPSocket_h

#include <iostream>

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <netdb.h>
#endif

class TCPSocket  
{
public:
  TCPSocket();
  ~TCPSocket();

  int OpenSocket(const char* IPadress, int port);
  void CloseSocket();
  int SendMessage(std::string message);

private:

  int       m_Sock;
  struct    sockaddr_in  m_Sin;
  
};

#endif
