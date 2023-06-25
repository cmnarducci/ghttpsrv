#ifndef __SRV_SOCK_H__
#define __SRV_SOCK_H__

#include "base_sock.h"

class SrvSock : public BaseSock
{
public:
   SrvSock();
   virtual ~SrvSock();

   class AcceptedSock : public BaseSock
   {
   public:
#ifdef HAVE_SSL
      AcceptedSock(SOCKET new_sock, SSL *ssl, const std::string& host, const std::string& sni = "");
#else
      AcceptedSock(SOCKET new_sock, const std::string& host);
#endif
      ~AcceptedSock();
      std::string GetHost() { return _host; };
#ifdef HAVE_SSL
      std::string GetSNI() { return _sni; };
#endif

   private:
      std::string _host;
#ifdef HAVE_SSL
      std::string _sni;
#endif
   };

   bool StartServer(const std::string& service, const std::string& srcip = "0.0.0.0");
   void StopServer();
   AcceptedSock *ClientAccept();

private:
   static uint8_t const _dhparam[];
};

#endif
