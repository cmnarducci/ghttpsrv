#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <cerrno>
#include <string>
#include <vector>
#include <sstream>
#include <cstdint>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif
#ifdef MINGW
#define WIN32_LEAN_AND_MEAN
#define __WINCRYPT_H__
#ifdef HAVE_SSL
#include <winsock2.h>
#else
#include <windows.h>
#endif
#endif
#ifdef HAVE_SSL
#include <openssl/ssl.h>
#endif
#if defined(HAVE_MD5)
#include "md5.h"
#elif defined(HAVE_SSL)
#include <openssl/md5.h>
#endif
#ifdef HAVE_AES
#include "aes.h"
#endif
#if defined (DEBUG) || defined(SOCK_DEBUG)
#include <iostream>

class mt_cout
{
public:
#ifdef HAVE_PTHREAD
   mt_cout() : _id(0) { pthread_mutex_init(&_cout_mutex, 0); };
   ~mt_cout() { pthread_mutex_destroy(&_cout_mutex); };
#else
   mt_cout() {};
   ~mt_cout() {};
#endif

   mt_cout& operator<<(const std::string& str)
   {
      _lock();
      std::cout << str;
      _unlock(str[str.length()-1]);
      return *this;
   };
   mt_cout& operator<<(const char ch)
   {
      _lock();
      std::cout << ch;
      _unlock(ch);
      return *this;
   };
   mt_cout& operator<<(std::ostream& (*pf)(std::ostream&))
   {
      _lock();
      std::cout << pf;
      if (pf == (std::ostream& (*)(std::ostream&))std::endl)
         _unlock('\n');
      return *this;
   };
   mt_cout& operator<<(const int num)
   {
      _lock();
      std::cout << num;
      return *this;
   };
   mt_cout& operator<<(const unsigned int num)
   {
      _lock();
      std::cout << num;
      return *this;
   };
   mt_cout& operator<<(const long long num)
   {
      _lock();
      std::cout << num;
      return *this;
   };

private:
#ifdef HAVE_PTHREAD
   pthread_t _id;
   pthread_mutex_t _cout_mutex;
#endif

   void _lock()
   {
#ifdef HAVE_PTHREAD
      int errcode = errno;
      if (_id != pthread_self())
      {
         pthread_mutex_lock(&_cout_mutex);
         _id = pthread_self();
      }
      errno = errcode; 
#endif
   };
   void _unlock(char last)
   {
#ifdef HAVE_PTHREAD
      if (last == '\n')
      {
         int errcode = errno;
         _id = 0;
         pthread_mutex_unlock(&_cout_mutex);
         errno = errcode; 
      }
#else
      (void)last;
#endif
   };
};
#endif

template <typename T>
class TokenIterator
{
   bool _end;
   std::string::size_type _current;
   std::string _data;
   std::string _delimiters;
   std::string _token;
   bool _empty;

   T convert(std::string v) const
   {
      T r;
      std::istringstream iss(v);
      iss >> r;
      return r;
   };
   std::string next()
   {
      std::string r;
      if (_current == std::string::npos)
      {
         _end = true;
         return r;
      }
      std::string::size_type pos;
      if (_empty)
      {
         pos = _data.find_first_of(_delimiters, _current);
      }
      else
      {
         _current = _data.find_first_not_of(_delimiters, _current);
         if (_current == std::string::npos)
         {
            _end = true;
            return r;
         }
         pos = _data.find_first_of(_delimiters, _current);
      }
      if (pos == std::string::npos)
      {
         r = _data.substr(_current);
         _current = std::string::npos;
      }
      else
      {
         r = _data.substr(_current, pos - _current);
         _current = pos + 1;
      }
      return r;
   };
   void Init(const char *s, const char *delimiters, bool empty)
   {
      _current = 0;
      _data = s;
      _end = (_data.length() == 0);
      _empty = empty;
      if (delimiters)
         _delimiters = delimiters;
      _token = next();
   };

public:
   TokenIterator(const char *s, const char *delimiters = " \t\b\n\"'`~!@#$%^&*-+=|\\/?.,;:(){}[]<>", bool empty = false)
   {
      Init(s, delimiters, empty);
   };
   TokenIterator(const std::string& s, const char *delimiters = " \t\b\n\"'`~!@#$%^&*-+=|\\/?.,;:(){}[]<>", bool empty = false)
   {
      Init(s.c_str(), delimiters, empty);
   };
   ~TokenIterator()
   {
   };
   TokenIterator& operator=(const std::string& s)
   {
      _current = 0;
      _data = s;
      _end = (_data.length() == 0);
      _token = next();
      return *this;
   };
   TokenIterator(const TokenIterator& old)
   {
      *this = old;
   };
   TokenIterator& operator=(const TokenIterator& r)
   {
      _end = r._end; _current = r._current;
      _data = r._data; _delimiters = r._delimiters;
      _empty = r._empty;
      _token = r._token;
      return *this;
   };
   bool operator==(const TokenIterator& r) const
   {
      return _end == r._end && _current == r._current && _data == r._data && _empty == r._empty;
   };
   bool operator!=(const TokenIterator& r) const
   {
      return !(*this == r);
   };
   T operator*() const
   {
      return convert(_token);
   };
   TokenIterator& operator++()
   {
      _token = next();
      return *this;
   };
   TokenIterator operator++(int)
   {
      TokenIterator c(*this);
      ++*this;
      return c;
   };
   operator bool() const
   {
      return !_end;
   };
};

template <>
inline const char *TokenIterator<const char *>::convert(std::string) const
{
   return "<char *> invalid template type";
}

template <>
inline std::string TokenIterator<std::string>::convert(std::string v) const
{
   return v;
}

class Tools
{
public:
   Tools() {};
   virtual ~Tools() {};

#if defined (HAVE_SSL) || defined(HAVE_MD5)
   static std::string GeneratePassword(const std::string& plain);
#endif
#if defined (HAVE_SSL) || (defined(HAVE_MD5) && defined(HAVE_AES))
   static void AESEncrypt(const uint8_t *key, const std::vector<uint8_t>& plain, std::vector<uint8_t>& data);
   static void AESDecrypt(const uint8_t *key, const std::vector<uint8_t>& data, std::vector<uint8_t>& plain);
   static void AESEncrypt(const std::string& pass, const std::vector<uint8_t>& plain, std::vector<uint8_t>& data);
   static void AESDecrypt(const std::string& pass, const std::vector<uint8_t>& data, std::vector<uint8_t>& plain);
   static std::string EncryptPassword(const std::string& pwd);
   static std::string DecryptPassword(const std::string& pwd);
#endif
   static std::string IPv4ToString(const uint32_t addr);
   static uint32_t StringToIPv4(const std::string& addr);
   static std::string PrefixToNetmask(const std::string& prefix);
   static std::string NetmaskToPrefix(const std::string& netmask);
   static bool IsInSubnet(const std::string& host, const std::string& net, const std::string& prefix);
   static std::string hexdump(const uint8_t *buf, int len, bool upper = false, bool format = false, bool space = false);
   static void hex2bin(const std::string& hexdata, uint8_t *bin);
   static std::string Trim(const std::string& item);
   static std::string GetErrorMessage(int err);
   static void Sleepms(unsigned long ms);
   static std::string Basename(const std::string& path);
#ifdef MINGW
   static std::string Unicode2Ascii(uint8_t *data, int len);
#endif
#if defined (DEBUG) || defined(SOCK_DEBUG)
   static mt_cout debug;
#endif
   class Base64
   {
      static inline bool is_base64(uint8_t c)
      {
         return (isalnum(c) || (c == '+') || (c == '/'));
      };
      static const std::string base64_chars;
   public:
      static std::string Encode(const uint8_t *bytes, unsigned int len);
      static std::string Encode(const std::string& str);
      static std::string Decode(const std::string& str);
      static std::vector<uint8_t> DecodeBin(const std::string& str);
   };

private:
#if defined (HAVE_SSL) || defined(HAVE_MD5)
   static void GenerateKey(uint8_t *key);
#endif
};

#endif
