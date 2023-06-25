#ifndef __BASE_SOCK_H__
#define __BASE_SOCK_H__

#include <cerrno>
#include <string>
#include <vector>
#include <map>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef MINGW
#define WIN32_LEAN_AND_MEAN
#define __WINCRYPT_H__
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>
#endif
#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif
#ifdef HAVE_SSL
#include <openssl/ssl.h>
#ifdef HAVE_PTHREAD
struct CRYPTO_dynlock_value { pthread_rwlock_t rwlock; };
#endif
#endif
#ifdef MINGW
#define __lasterror      WSAGetLastError()
#define __seterror(x)    WSASetLastError(x)
#define __poll           WSAPoll
#define __EWOULDBLOCK    WSAEWOULDBLOCK
#define __EINPROGRESS    WSAEINPROGRESS
#define __ETIMEDOUT      WSAETIMEDOUT
#define __EISCONN        WSAEISCONN
#define __ENOTCONN       WSAENOTCONN
#define __EADDRNOTAVAIL  WSAEADDRNOTAVAIL
#define __ECONNABORTED   WSAECONNABORTED
#define __ECONNRESET     WSAECONNRESET
#define __EINTR          WSAEINTR
#define __EAGAIN         WSAEWOULDBLOCK
#define __EINVAL         WSAEINVAL
#else
#define __lasterror      errno
#define __seterror(x)    errno = x
#define __poll           poll
#define closesocket      close
#define ioctlsocket      ioctl
#define SOCKET           int
#define __EWOULDBLOCK    EWOULDBLOCK
#define __EINPROGRESS    EINPROGRESS
#define __ETIMEDOUT      ETIMEDOUT
#define __EISCONN        EISCONN
#define __ENOTCONN       ENOTCONN
#define __EADDRNOTAVAIL  EADDRNOTAVAIL
#define __ECONNABORTED   ECONNABORTED
#define __ECONNRESET     ECONNRESET
#define __EINTR          EINTR
#define __EAGAIN         EAGAIN
#define __EINVAL         EINVAL
#endif

class BaseSock
{
public:
#ifdef HAVE_SSL
   enum SSLMethod
   {
      SSLv23   = 0x01,
      TLSv1    = 0x02,
      TLSv1_1  = 0x04,
      TLSv1_2  = 0x08,
      DTLSv1   = 0x10,
      DTLSv1_2 = 0x20,
      TLS      = 0x40,
      DTLS     = 0x80
   };
#endif
   BaseSock();
   virtual ~BaseSock();

   virtual int ReadData(uint8_t *buf, int len, bool peek = false);
   virtual int ReadDataExact(uint8_t *buf, int len);
   virtual bool ReadLine(std::string& line);
   virtual int WriteData(const uint8_t *buf, int len);
   virtual bool WriteLine(const std::string& line);
   virtual int BytesAvailable();
   virtual std::string GetErrorMessage();
   virtual int GetErrorNumber() { return __lasterror; };
   virtual int SetTimeOut(int nTO);
   virtual void CloseConnection();
   virtual SOCKET GetSocket() { return _sock; };
   static void Setup();
   static void Cleanup();
   int GetReceiveBufferSize();
   void SetReceiveBufferSize(int size);
   void SetTcpNoDelay(bool on = true);
   int GetSendBufferSize();
   void SetSendBufferSize(int size);
#ifdef HAVE_SSL
   virtual bool GetSSL() { return _sslsock; };
   virtual bool SetSSL(bool ssl, bool use_cert = false, SSLMethod method = TLS, const std::string& ca_file = "",
                                                                                const std::string& crl_file = "");
   virtual bool StartSSL(bool use_cert = false, SSLMethod method = TLS);
   bool UseCertificate(const std::string& cert_file, const std::string& pk_file = "", const std::string& pwd = "");
   std::string GetPeerCertificateCommonName();
   std::string GetPeerCertificateCN();
   std::string GetPeerCertificateO();
   std::string GetPeerCertificateOU();
   std::string GetPeerCertificateEmail();
   std::string GetPeerCertificateExpiry();
   std::string GetPeerCertificateIssuer();
   std::string GetSSLVersion();
   void SetSNI(const std::string& sni) { _sni = sni; };
#endif
   static std::string GetHostname();
   static int GetLocalAddresses(std::map<std::string,std::vector<std::string> >& ips);
   static uint32_t GetIPv4(const std::string& host);

protected:
   enum DataAction { DATA_READ, DATA_WRITE };
   int LowLevelRead(uint8_t *buf, int len, bool peek = false);
   int LowLevelWrite(const uint8_t *buf, int len);
   int ExchangeData(DataAction action, uint8_t *buf, int len, bool peek = false);
   int FindEOL(const uint8_t *buffer, int len, bool *crlf);

   SOCKET _sock;
   int _TimeOut;
#ifdef HAVE_SSL
#if OPENSSL_VERSION_NUMBER >= 0x10100000
   static const SSLMethod _best_method = TLS;
   static const uint8_t _method_mask = 0xFF;
#elif OPENSSL_VERSION_NUMBER >= 0x10001000
   static const SSLMethod _best_method = TLSv1_2;
   static const uint8_t _method_mask = 0x1F;
#elif OPENSSL_VERSION_NUMBER >= 0x00900000
   static const SSLMethod _best_method = TLSv1;
   static const uint8_t _method_mask = 0x13;
#else
   static const SSLMethod _best_method = SSLv3;
   static const uint8_t _method_mask = 0x01;
#endif
   bool _sslsock;
   SSL_CTX *_ctx;
   SSL *_ssl;
   const int _ssl_id_context;
   static uint8_t _pk_der[];
   static uint8_t _cert_der[];
   std::string _cert_file;
   std::string _pk_file;
   std::string _pk_pwd;
   std::string _sni;
#endif
private:
#ifdef HAVE_SSL
   static int CertKeyPasswordCB(char *buf, int size, int rwflag, void *password);
   static int CertVerifyCB(int preverify_ok, X509_STORE_CTX *x509_ctx);
   static int ServerNameIndicationCB(SSL *ssl, int *ad, void *arg);
#ifdef HAVE_PTHREAD
   static void MultiThreadSetup();
   static void MultiThreadCleanup();
   static CRYPTO_dynlock_value *ssl_dyn_create(const char *, int);
   static void ssl_dyn_lock(int mode, CRYPTO_dynlock_value *cdv, const char *, int);
   static void ssl_dyn_destroy(CRYPTO_dynlock_value *cdv, const char *, int);
   static void ssl_locking(int mode, int index, const char *, int);
   static pthread_rwlock_t *_locks;
#endif
#endif
};

#endif
