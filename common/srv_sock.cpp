#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <cstring>
#include <csignal>
#ifdef SOLARIS
#include <sys/filio.h>
#endif
#include "srv_sock.h"

#if defined(HAVE_SSL) && OPENSSL_VERSION_NUMBER >= 0x10100000
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

uint8_t const SrvSock::_dhparam[] =
{
   0x30, 0x82, 0x01, 0x08, 0x02, 0x82, 0x01, 0x01, 0x00, 0xfb, 0x29, 0x1a,
   0x3f, 0xa7, 0x46, 0x8f, 0x11, 0x06, 0x50, 0x20, 0xd5, 0x61, 0xb2, 0x27,
   0x36, 0x1c, 0x56, 0x69, 0x01, 0xbf, 0x52, 0x86, 0x62, 0xe0, 0x43, 0xc4,
   0x86, 0x7a, 0x91, 0xf9, 0x3c, 0xfa, 0xec, 0x24, 0xaf, 0xa5, 0xbf, 0x31,
   0xef, 0x95, 0x3e, 0xb0, 0xbe, 0xf3, 0xd9, 0xe0, 0x73, 0xcd, 0xaa, 0xd1,
   0x32, 0x06, 0x70, 0xd1, 0x6d, 0xdd, 0xdf, 0xe6, 0x77, 0xb8, 0x0e, 0x2d,
   0x4b, 0x59, 0x57, 0x1a, 0xa4, 0xa2, 0x6e, 0x43, 0xbd, 0x39, 0x88, 0xcf,
   0xf2, 0x2b, 0x6e, 0x3c, 0xfc, 0xc3, 0x92, 0x76, 0x52, 0xe5, 0x27, 0x9b,
   0x71, 0x82, 0x4c, 0x00, 0x70, 0x1a, 0x0f, 0xda, 0xdd, 0x23, 0xf9, 0xbd,
   0x62, 0x89, 0xea, 0x96, 0x35, 0xf1, 0xf2, 0x81, 0x22, 0x10, 0x5a, 0x2f,
   0x8f, 0xa5, 0x48, 0x89, 0x40, 0xa3, 0x59, 0xe8, 0x44, 0xad, 0x1e, 0xa9,
   0x1a, 0xc2, 0x9f, 0x39, 0x57, 0x60, 0xe9, 0xe3, 0x83, 0xf4, 0x94, 0x4e,
   0xb3, 0x5f, 0x63, 0xcd, 0xde, 0xb7, 0xef, 0xe1, 0x81, 0x2b, 0x6a, 0xbe,
   0x38, 0x3a, 0xfd, 0x33, 0x31, 0x1c, 0x16, 0x9c, 0x98, 0xfc, 0xba, 0x86,
   0xc1, 0xff, 0x5b, 0x40, 0x6e, 0xf9, 0x4e, 0xe7, 0xb7, 0xc7, 0x3f, 0xad,
   0x2d, 0x91, 0x09, 0xb6, 0x6c, 0x94, 0xfb, 0xf9, 0x4a, 0x4f, 0xf9, 0xcb,
   0x90, 0x40, 0x8f, 0x7b, 0x2e, 0x0f, 0x5c, 0x78, 0xd5, 0xa6, 0xb4, 0x45,
   0x3e, 0x6f, 0xfd, 0xe2, 0xdd, 0xf8, 0xde, 0x5b, 0xa7, 0xe9, 0x61, 0xc6,
   0x5f, 0x65, 0x9b, 0x13, 0xf2, 0x38, 0xd6, 0x24, 0xd8, 0x4f, 0xec, 0x18,
   0xed, 0xbf, 0x16, 0x7f, 0xe4, 0xe0, 0xed, 0x32, 0x55, 0x38, 0xdb, 0xa6,
   0xdb, 0xfe, 0x8a, 0xd9, 0xc8, 0xc9, 0x81, 0x63, 0x68, 0xab, 0x14, 0xf7,
   0x17, 0x17, 0xe1, 0x4a, 0x4b, 0x61, 0x94, 0x56, 0x26, 0xf1, 0xdd, 0x5c,
   0x93, 0x02, 0x01, 0x02
};

using namespace std;

SrvSock::SrvSock() : BaseSock()
{
}

SrvSock::~SrvSock()
{
   StopServer();
}

bool SrvSock::StartServer(const string& service, const string& srcip)
{
   if (_sock != (SOCKET)-1)
   {
      __seterror(__EISCONN);
      return false;
   }
   addrinfo *srchost;
   addrinfo hints;

   __seterror(0);
   memset(&hints, 0, sizeof(hints));
   hints.ai_family = PF_INET;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_PASSIVE;
   const char *src = (srcip == "0.0.0.0") ? 0 : srcip.c_str();
   if (getaddrinfo(src, service.c_str(), &hints, &srchost) != 0)
   {
      __seterror(__EADDRNOTAVAIL);
      return false;
   }
   if ((_sock = socket(srchost->ai_family, srchost->ai_socktype, 0)) == (SOCKET)-1)
   {
      freeaddrinfo(srchost);
      return false;
   }
   int opt = 1;
   setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(int));
   if (::bind(_sock, srchost->ai_addr, srchost->ai_addrlen) < 0 || listen(_sock, SOMAXCONN) < 0)
   {
      StopServer();
      freeaddrinfo(srchost);
      return false;
   }
   unsigned long nb_opt = 1;
   ioctlsocket(_sock, FIONBIO, &nb_opt);
   freeaddrinfo(srchost);
   return true;
}

void SrvSock::StopServer()
{
   CloseConnection();
}

SrvSock::AcceptedSock *SrvSock::ClientAccept()
{
   AcceptedSock *new_conn = 0;
   pollfd fds[1] = { { _sock, POLLIN, 0 } };
   int rc;
   do
   {
      rc = __poll(fds, sizeof(fds) / sizeof(fds[0]), _TimeOut * 1000);
   } while (rc == -1 && __lasterror == __EINTR);
   if (rc > 0 && fds[0].revents & POLLIN)
   {
      struct sockaddr_in saddr;
      socklen_t slen = sizeof(saddr);
      SOCKET new_sock;
      do
      {
         new_sock = accept(_sock, (sockaddr *)&saddr, &slen);
      } while (new_sock == (SOCKET)-1 && __lasterror == __EINTR);
      if (new_sock != (SOCKET)-1)
      {
#ifdef HAVE_SSL
         SSL *ssl = 0;
         if (_sslsock)
         {
            SSL_CTX_set_session_id_context(_ctx, (const unsigned char *)&_ssl_id_context, sizeof(_ssl_id_context));
            if ((ssl = SSL_new(_ctx)) != 0)
            {
#if OPENSSL_VERSION_NUMBER >= 0x30000000
#if defined(OPENSSL_NO_EC)
               SSL_set1_groups_list(ssl, "ffdhe3072");
#else
               SSL_set1_groups_list(ssl, "P-256");
#endif
#elif OPENSSL_VERSION_NUMBER >= 0x10000000 && !defined(OPENSSL_NO_EC)
               EC_KEY *ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
               if (ecdh)
               {
                  SSL_set_tmp_ecdh(ssl, ecdh);
                  EC_KEY_free(ecdh);
               }
               SSL_set_options(ssl, SSL_OP_SINGLE_ECDH_USE);
#endif
#if OPENSSL_VERSION_NUMBER < 0x30000000
               const unsigned char *ptr = _dhparam;
               DH *dh = d2i_DHparams(0, &ptr, sizeof(_dhparam));
               if (dh)
               {
                  SSL_set_tmp_dh(ssl, dh);
                  DH_free(dh);
               }
               SSL_set_options(ssl, SSL_OP_SINGLE_DH_USE);
#endif
               SSL_set_fd(ssl, new_sock);
               SSL_set_accept_state(ssl);
               unsigned long opt = 0;
               ioctlsocket(new_sock, FIONBIO, &opt);
               struct timeval tv;
               tv.tv_sec = _TimeOut * 1000;
               tv.tv_usec = 0;
               setsockopt(new_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof tv);
               int rc;
               if ((rc = SSL_accept(ssl)) <= 0)
               {
                  int err = SSL_get_error(ssl, rc);
                  struct linger nolinger;
                  nolinger.l_onoff = 1;
                  nolinger.l_linger = 0;
                  setsockopt(new_sock, SOL_SOCKET, SO_LINGER, (char *)&nolinger, sizeof(nolinger));
                  SSL_shutdown(ssl);
                  SSL_free(ssl);
                  closesocket(new_sock);
                  ssl = 0;
                  __seterror((err == SSL_ERROR_SYSCALL) ? __ETIMEDOUT : __ECONNABORTED);
                  return 0;
               }
               opt = 1;
               ioctlsocket(new_sock, FIONBIO, &opt);
            }
            else
            {
               closesocket(new_sock);
               __seterror(__ECONNABORTED);
            }
         }
         if (ssl || !_sslsock)
            new_conn = new AcceptedSock(new_sock, ssl, string(inet_ntoa(saddr.sin_addr)), _sni);
#else
         unsigned long opt = 1;
         ioctlsocket(new_sock, FIONBIO, &opt);
         new_conn = new AcceptedSock(new_sock, string(inet_ntoa(saddr.sin_addr)));
#endif
      }
   }
   else if (rc > 0 && fds[0].revents & POLLHUP)
      __seterror(__ECONNABORTED);
   else if (rc == 0)
      __seterror(__ETIMEDOUT);

   return new_conn;
}

SrvSock::AcceptedSock::~AcceptedSock()
{
   CloseConnection();
}

#ifdef HAVE_SSL
SrvSock::AcceptedSock::AcceptedSock(SOCKET new_sock, SSL *ssl, const string& host, const string& sni)
#else
SrvSock::AcceptedSock::AcceptedSock(SOCKET new_sock, const string& host)
#endif
{
   AcceptedSock::_sock = new_sock;
   AcceptedSock::_host = host;
#ifdef HAVE_SSL
   AcceptedSock::_ssl = ssl;
   AcceptedSock::_sni = sni;
   if (ssl)
      AcceptedSock::_sslsock = true;
#endif
}
