#include "TCPSocket.h"

TCPSocket::TCPSocket()
{
}

TCPSocket::~TCPSocket()
{
}

int TCPSocket::OpenSocket(const char* IPadress, int port)
{
  #ifdef WIN32
    WSADATA    WsaData;
    int err = WSAStartup (0x0101, &WsaData);              // Init Winsock
    if(err!=0)
        return -1;
  #endif

  m_Sock = socket (AF_INET, SOCK_STREAM, 0);

  if (m_Sock < 0)
    {
    return -1;
    }
    
  m_Sin.sin_family = AF_INET;  
  m_Sin.sin_port = htons( (unsigned short)port );
  m_Sin.sin_addr.s_addr = inet_addr(IPadress);
  
  if( connect (m_Sock,(struct sockaddr* )&m_Sin, sizeof(m_Sin) ) )
  {
     //std::cerr << "ERROR: Connexion problem " << std::endl;
     return -1;
  }
  return 0;
}

void TCPSocket::CloseSocket()
{
  #ifdef WIN32
    closesocket(m_Sock);
  #else
    close(m_Sock);  
  #endif
}

int TCPSocket::SendMessage(std::string message)
{
  if( send(m_Sock,message.c_str(),message.length(),0) == -1 )
    {
    return -1;
    }
  return 0;
}
