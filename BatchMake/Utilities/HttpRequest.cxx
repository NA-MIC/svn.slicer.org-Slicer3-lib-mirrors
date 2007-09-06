#include "HttpRequest.h"

HttpRequest::HttpRequest()
{
  m_ParamList.clear();
}

HttpRequest::~HttpRequest()
{
  std::vector<Paramstruct>::iterator it =  m_ParamList.begin();
  while(it != m_ParamList.end())
    {
    Paramstruct ps = *it;
    delete [] ps.value;
    it = m_ParamList.erase(it);
    }
}

void HttpRequest::MemBufferCreate(MemBuffer *b)
{
   b->size = MEM_BUFFER_SIZE;
   b->buffer = new unsigned char[b->size];
   b->position = b->buffer;
}

void HttpRequest::MemBufferGrow(MemBuffer *b)
{
    size_t sz;
    sz = b->position - b->buffer;
    unsigned char* buffer = new unsigned char[b->size*2];
    memcpy(buffer,b->buffer,b->size);
    b->size = b->size *2;
    delete [] b->buffer;
    b->buffer = buffer;
    b->position = b->buffer + sz;  // readjust current position
}

void HttpRequest::MemBufferAddByte(MemBuffer *b,unsigned char byt)
{
  if( (size_t)(b->position-b->buffer) >= b->size )
    { 
    MemBufferGrow(b);
    }
  *(b->position++) = byt;
}

void HttpRequest::MemBufferAddBuffer(MemBuffer *b,
                    unsigned char *buffer, size_t size)
{
    while( ((size_t)(b->position-b->buffer)+size) >= b->size )
        MemBufferGrow(b);

    memcpy(b->position,buffer,size);
    b->position+=size;
}

std::string HttpRequest::GetHostName()
{
#ifdef WIN32
    WSADATA    WsaData;
    int err = WSAStartup (0x0101, &WsaData);              // Init Winsock
    if(err!=0)
        return "";
#endif
  char nameBuffer[1024];
  gethostname(nameBuffer, 1024);
  std::string hostName(nameBuffer);
  return hostName;
}

std::string HttpRequest::GetHostIp()
{
  #ifdef WIN32
    WSADATA    WsaData;
    int err = WSAStartup (0x0101, &WsaData);              // Init Winsock
    if(err!=0)
        return "";
  #endif
 
  struct hostent *phe = gethostbyname(GetHostName().c_str());
  if (phe == 0) 
      return "";

  struct in_addr addr;
  char** address = phe->h_addr_list;
  int m_numaddrs = 0;
  while (*address)
  {
    m_numaddrs++;
    address++;
  }

  std::string m_ip = "";
  if (m_numaddrs != 0)
  {
    memcpy(&addr, phe->h_addr_list[m_numaddrs-1], sizeof(struct in_addr));
    m_ip = inet_ntoa(addr);
  }

 return m_ip;
}


int HttpRequest::GetHostAddress(char* host)
{
    struct hostent *phe;
    char *p;

    phe = gethostbyname( host );
            
    if(phe==NULL)
        return 0;
    
    p = *phe->h_addr_list;
    return *((int*)p);
}

void HttpRequest::SendString(int sock,std::string str)
{
    send(sock,str.c_str(),str.length(),0);
}

bool HttpRequest::ValidHostChar(char ch)
{
    return( isalpha(ch) || isdigit(ch)
        || ch=='-' || ch=='.' || ch==':' );
}


void HttpRequest::ParseURL(const char* url,char* protocol,int lprotocol,
        char* host,int lhost,char* request,int lrequest,int *port)
{
    char *work,*ptr,*ptr2;

    *protocol = *host = *request = 0;
    *port=80;

    work = strdup(url);
    //strupr(work);
    for (unsigned int i=0;i<strlen(work);i++)
    {
      work[i] = toupper(work[i]);
    }

    ptr = strchr(work,':');              // find protocol if any
    if(ptr!=NULL)
    {
        *(ptr++) = 0;
        strncpy(protocol,work,lprotocol);
    }
    else
    {
        strncpy(protocol,"HTTP",lprotocol);
        ptr = work;
    }

    if( (*ptr=='/') && (*(ptr+1)=='/') )      // skip past opening /'s 
        ptr+=2;

    ptr2 = ptr;                    // find host
    while( ValidHostChar(*ptr2) && *ptr2 )
        ptr2++;

    *ptr2=0;
    strncpy(host,ptr,lhost);

    strncpy(request,url + (ptr2-work),lrequest);  // find the request

    ptr = strchr(host,':');              // find the port number, if any
    if(ptr!=NULL)
    {
        *ptr=0;
        *port = atoi(ptr+1);
    }

    free(work);
}

int HttpRequest::SendHTTP(const char*  url,char*  headerReceive,unsigned char *post,
        unsigned int postLength,MessageStruct *req)
{
    struct    sockaddr_in  sin;
    int        sock;
    char      buffer[512];
    char      protocol[20],host[256],request[1024];
    int        l,port,chars,err;
    MemBuffer    headersBuffer,messageBuffer;
    char      headerSend[1024];
    bool      done;
    
    ParseURL(url,protocol,sizeof(protocol),host,sizeof(host),    // Parse the URL
        request,sizeof(request),&port);
    
    if(strcmp(protocol,"HTTP"))
        return 1;

#ifdef WIN32
    WSADATA    WsaData;
    err = WSAStartup (0x0101, &WsaData);              // Init Winsock
    if(err!=0)
        return 1;
#endif

    sock = socket (AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
        return 1;

    
  sin.sin_family = AF_INET;  
  sin.sin_port = htons( (unsigned short)port );
  sin.sin_addr.s_addr = GetHostAddress(host);

  if( connect (sock,(struct sockaddr* )&sin, sizeof(sin) ) )
  {
     //std::cerr << "ERROR: Connexion problem " << std::endl;
     return 1;
  }

  if( !*request )
      strncpy(request,"/",sizeof(request));

  if( post == NULL )
  {
    SendString(sock,"GET ");
    strcpy(headerSend, "GET ");
  }
    else 
    {
    SendString(sock,"POST ");
    strcpy(headerSend, "POST ");
    }
    SendString(sock,request);
    strcat(headerSend, request);

    SendString(sock," HTTP/1.0\r\n");
    strcat(headerSend, " HTTP/1.0\r\n");

    SendString(sock,"Accept: image/gif, image/x-xbitmap,"
        " image/jpeg, image/pjpeg, application/vnd.ms-excel,"
        " application/msword, application/vnd.ms-powerpoint,"
        " */*\r\n");
    strcat(headerSend, "Accept: image/gif, image/x-xbitmap,"
        " image/jpeg, image/pjpeg, application/vnd.ms-excel,"
        " application/msword, application/vnd.ms-powerpoint,"
        " */*\r\n");

    SendString(sock,"Accept-Language: en-us\r\n");
    strcat(headerSend, "Accept-Language: en-us\r\n");

    SendString(sock,"Accept-Encoding: gzip, deflate\r\n");
    strcat(headerSend, "Accept-Encoding: gzip, deflate\r\n");

    SendString(sock,"User-Agent: Mozilla/4.0\r\n");
    strcat(headerSend, "User-Agent: Mozilla/4.0\r\n");

    SendString(sock,"Host: ");
    strcat(headerSend, "Host: ");

    SendString(sock,host);
    strcat(headerSend, host);

    SendString(sock,"\r\n");
    strcat(headerSend, "\r\n");
    if(postLength)
      {
      sprintf(buffer,"Content-Length: %ld\r\n",postLength-4);
      SendString(sock,buffer);
      strcat(headerSend, buffer);
      }

    if( (headerReceive!=NULL) && *headerReceive )
      {
      SendString(sock,headerReceive);
      strcat(headerSend, headerReceive);
      }
    
    SendString(sock,"\r\n");                // Send a blank line to signal end of HTTP headerReceive
    strcat(headerSend, "\r\n");

    if( (post!=NULL) && postLength )
      {
      long int remaining = postLength;
      do
        {
        long int n = send(sock,(const char*)post,postLength,0);
        post += n;
        remaining -= n;
        } while(remaining>0);
      }

 /*
  std::string test = "";

  std::string line = "Content-Type: multipart/form-data; boundary=\"123456789123456789\"\r\n";
  long prev = test.size();
  test += line;
  std::cout << test.size()-prev << std::endl;
  prev = test.size();
  test += "\r\n";
  std::cout << test.size()-prev << std::endl; 
  prev = test.size();
  std::string test2 = "--123456789123456789\r\n";
  test2 += "Content-Disposition: form-data; name=\"gender\"\r\n";
  test2 += "\r\n";
  test2 += "M\r\n";
  test2 += "--123456789123456789\r\n";
  test2 += "Content-Disposition: form-data; name=\"born\"\r\n";
  test2 += "\r\n";
  test2 += "1964\r\n";
  test += test2;
  test += "--123456789123456789\r\n";
  std::cout << test.size()-prev << std::endl; 
  prev = test.size();
  test += "Content-Disposition: form-data; name=\"init\"; filename=\".profile\"\r\n";
  std::cout << test.size()-prev << std::endl; 
  prev = test.size();
  test += "Content-Type: text/plain\r\n";
  std::cout << test.size()-prev << std::endl; 
  prev = test.size();
  test += "\r\n";
  std::cout << test.size()-prev << std::endl; 
  prev = test.size();
  test += "PATH=/local/perl/bin:$PATH\r\n";
  std::cout << test.size()-prev << std::endl; 
  prev = test.size();
  test += "\r\n";
  std::cout << test.size()-prev << std::endl; 
  prev = test.size();
  test += "--123456789123456789--\r\n";
  std::cout << test.size()-prev << std::endl; 
  prev = test.size();

  char* testl = new char[100];
  std::cout << test.size() << std::endl;
  std::cout << "Total = " << test.size()-line.size() << std::endl;
  //std::cout << "Test 2 = " << test2.size() << std::endl;
  sprintf(testl,"Content-Length: %d\r\n",test.size()-line.size()-6); //138
  std::cout << testl <<  std::endl;
  std::string sending = testl;
  sending += test;

  SendString(sock,sending.c_str());
*/

    // Read the result
    MemBufferCreate(&headersBuffer );
    chars = 0;
    done = false;

    while(!done)
    {
        l = recv(sock,buffer,1,0);
        if(l<0)
            done=true;

        switch(*buffer)
        {
            case '\r':
                break;
            case '\n':
                if(chars==0)
                    done = true;
                chars=0;
                break;
            default:
                chars++;
                break;
        }

        MemBufferAddByte(&headersBuffer,*buffer);
    }

    req->headerReceive  = (char*) headersBuffer.buffer;
    *(headersBuffer.position) = 0;

    

    MemBufferCreate(&messageBuffer);              // Now read the HTTP body

    do
    {
        l = recv(sock,buffer,sizeof(buffer)-1,0);
        if(l<0)
            break;
        *(buffer+l)=0;
        MemBufferAddBuffer(&messageBuffer, (unsigned char*)&buffer, l);
    } while(l>0);
    *messageBuffer.position = 0;
    
    req->message = (const char*) messageBuffer.buffer;
 
    shutdown(sock,2);
#ifdef WIN32
    closesocket(sock);
#else
    close(sock);  
#endif
  
  // Deleting memory
  delete [] headersBuffer.buffer;
  delete [] messageBuffer.buffer;
  return 0;
}

std::string HttpRequest::CreateFile(std::string name,std::string filename)
{
  FILE* m_file;
  m_file = fopen(filename.c_str(),"rb");

  if (m_file == 0)
    {
    return "";
    }

  std::string m_text = "--29772313742745\n";
  m_text += "Content-Disposition: form-data; name=\"MAX_FILE_SIZE\"\n";
  m_text += "\n";
  m_text += "10000000\n";
  m_text += "--29772313742745\n";
  m_text += "Content-Disposition: form-data; name=\"";
  m_text += name;
  m_text += "\"; filename=\"";
 
  if (filename.rfind("/") != -1)
   m_text += filename.substr(filename.rfind("/")+1);
  else
   if (filename.rfind("\\") != -1)
      m_text += filename.substr(filename.rfind("\\")+1);
    else
      m_text += filename;

  m_text += "\"\r\n";
  m_text += "Content-Type: text/plain\r\n";
  m_text += "\r\n";
  unsigned char m_val;
  while (!feof(m_file))
    {
    fread(&m_val,1,1,m_file);
    m_text += m_val;
    }  
  m_text += "\r\n";

  fclose(m_file);

  return m_text;
}

void HttpRequest::AddParam(std::string name,const char* value,unsigned long size)
{
  Paramstruct m_param;
  m_param.name = name;
  if(size==0)
    {
    size = strlen(value);
    }
  m_param.value = new char[size+1];
  strncpy(m_param.value,value,size);
  m_param.value[size] = '\0';
  m_param.size = size;
  std::string temp = m_param.value;

  m_ParamList.push_back(m_param);
}

void HttpRequest::AddFile(std::string name,std::string filename)
{
  FilePairType file;
  file.first = name;
  file.second = filename;
  m_Filenames.push_back(file);
}

std::string HttpRequest::Send(std::string url)
{
  MessageStruct  req;

  if ((m_ParamList.size() == 0) && (m_Filenames.size() == 0))
  {
    std::cerr << "No Param or File defined !" << std::endl;
    return "-2";
  }

  unsigned long textLenght = 0;

  std::string m_text;
  unsigned int i=0;
  for (i=0;i<m_ParamList.size();i++)
    {
    m_text += "--29772313742745\n";
    m_text += "Content-Disposition: form-data; name=\"";
    m_text += m_ParamList[i].name;
    m_text += "\"\r\n";
    m_text += "\r\n";
    m_text += m_ParamList[i].value;
    m_text += "\r\n";
    }
  for(i=0;i<m_Filenames.size();i++)
    {
    FilePairType p = m_Filenames[i];
    std::string m_file = CreateFile(p.first,p.second);
    if (m_file.length() == 0)
      {
      //return "-3";
      }
    else
      {
      m_text += m_file;
      }
    }

  
  m_text += "--29772313742745--\r\n";

  if (SendHTTP(  url.c_str(),
    "Content-Type: multipart/form-data; boundary=\"29772313742745\"\r\n",
    (unsigned char*)m_text.c_str(),
    m_text.length(),
    &req) == 1)
    {
    return "-1";
    }
 
  std::vector<Paramstruct>::iterator it =  m_ParamList.begin();
  while(it != m_ParamList.end())
    {
    Paramstruct ps = *it;
    delete [] ps.value;
    it = m_ParamList.erase(it);
    } 
 
  m_ParamList.clear();
  m_Filenames.clear();

  return req.message;
}
