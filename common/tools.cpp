#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <cstring>
#include <ctime>
#include <iomanip>
#include <sstream>
#ifdef HAVE_SSL
#include <openssl/evp.h>
#endif
#ifdef SOLARIS
#include <sys/filio.h>
#endif
#include "tools.h"

#if defined(HAVE_SSL) && OPENSSL_VERSION_NUMBER >= 0x10100000
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined (DEBUG) || defined(SOCK_DEBUG)
mt_cout Tools::debug;
#endif

using namespace std;

const string Tools::Base64::base64_chars =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
   "abcdefghijklmnopqrstuvwxyz"
   "0123456789+/";

string Tools::GetErrorMessage(int err)
{
   string errmsg;
#ifdef MINGW
   LPTSTR errTxt = 0;
   if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                     0, err, 0, (LPTSTR)&errTxt, 0, 0))
   {
#ifdef UNICODE
      errmsg = Unicode2Ascii((uint8_t *)errTxt, wcslen(errTxt));
#else
      errmsg = errTxt;
#endif
      LocalFree(errTxt);
      while (errmsg.at(errmsg.length()-1) == '\n' || errmsg.at(errmsg.length()-1) == '\r')
         errmsg = errmsg.substr(0, errmsg.length()-1);
   }
   else
   {
      errmsg = "No Error Description";
   }
#else
   errmsg = strerror(err);
#endif
   return errmsg;
}

#if defined (HAVE_SSL) || defined(HAVE_MD5)
string Tools::GeneratePassword(const string& plain)
{
   if (plain.length() == 0)
      return "";
   const uint8_t secret[] = { 0x62, 0x31, 0x2D, 0x6E, 0x65, 0x6F, 0x65, 0x6E, 0x00 };
   uint8_t hash[MD5_DIGEST_LENGTH];
   ostringstream oss;
   oss << plain << secret;
   MD5((uint8_t *)oss.str().c_str(), oss.str().length(), hash);
   for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
      hash[i] ^= secret[i % 2];
   string key = Base64::Encode(hash, MD5_DIGEST_LENGTH);
   return key.substr(0, key.length() - 2);
}
#endif

#if defined (HAVE_SSL) || (defined(HAVE_MD5) && defined(HAVE_AES))
void Tools::GenerateKey(uint8_t *key)
{
   if (key == 0)
      return;
   const uint8_t secret[] = { 0x62, 0x31, 0x2D, 0x6E, 0x65, 0x6F, 0x65, 0x6E, 0x00 };
   ostringstream oss;
   oss << secret;
   MD5((uint8_t *)oss.str().c_str(), oss.str().length(), key);
}

void Tools::AESEncrypt(const uint8_t *key, const vector<uint8_t>& plain, vector<uint8_t>& data)
{
   data.clear();
   if (plain.size() == 0)
      return;

   uint8_t buf[((plain.size() >> 4) + 1) << 4];
   uint8_t iv[16] = { 0x33, 0x33, 0 };
   memset(buf, 16 - (plain.size() % 16), sizeof(buf));
#ifdef HAVE_SSL
   EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
   int buflen;

   EVP_EncryptInit(ctx, EVP_aes_128_cbc(), key, iv);
   EVP_EncryptUpdate(ctx, buf, &buflen, &plain[0], plain.size());
   EVP_EncryptFinal(ctx, buf + buflen, &buflen);
   EVP_CIPHER_CTX_free(ctx);
#else
   struct AES_ctx ctx;
   AES_init_ctx_iv(&ctx, key, iv);
   memcpy(buf, &plain[0], plain.size());
   AES_CBC_encrypt_buffer(&ctx, buf, sizeof(buf));
#endif
   data.insert(data.begin(), buf, buf + sizeof(buf));
}

void Tools::AESDecrypt(const uint8_t *key, const vector<uint8_t>& data, vector<uint8_t>& plain)
{
   plain.clear();
   if (data.size() == 0)
      return;

   uint8_t buf[data.size()];
   uint8_t iv[16] = { 0x33, 0x33, 0 };
   memset(buf, 0, data.size());
#ifdef HAVE_SSL
   unsigned int len = 0;
   EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
   int buflen;

   EVP_DecryptInit(ctx, EVP_aes_128_cbc(), key, iv);
   EVP_DecryptUpdate(ctx, buf, &buflen, &data[0], data.size());
   len += buflen;
   EVP_DecryptFinal(ctx, buf + buflen, &buflen);
   len += buflen;
   EVP_CIPHER_CTX_free(ctx);
#else
   unsigned int len = sizeof(buf);
   struct AES_ctx ctx;
   AES_init_ctx_iv(&ctx, key, iv);
   memcpy(buf, &data[0], data.size());
   AES_CBC_decrypt_buffer(&ctx, buf, sizeof(buf));
   if (!(buf[len - 1] & 0xE0))
      len -= buf[len - 1];
#endif
   plain.insert(plain.begin(), buf, buf + len);
}

void Tools::AESEncrypt(const string& pass, const vector<uint8_t>& plain, vector<uint8_t>& data)
{
   uint8_t key[MD5_DIGEST_LENGTH];
   MD5((uint8_t *)pass.c_str(), pass.length(), key);
   AESEncrypt(key, plain, data);
}

void Tools::AESDecrypt(const string& pass, const vector<uint8_t>& data, vector<uint8_t>& plain)
{
   uint8_t key[MD5_DIGEST_LENGTH];
   MD5((uint8_t *)pass.c_str(), pass.length(), key);
   AESDecrypt(key, data, plain);
}

string Tools::EncryptPassword(const string& pwd)
{
   if (pwd.length() == 0)
      return "";
   uint8_t key[MD5_DIGEST_LENGTH];
   GenerateKey(key);
   vector<uint8_t> plain(pwd.begin(), pwd.end());
   vector<uint8_t> data;
   AESEncrypt(key, plain, data);
   return Base64::Encode(&data[0], data.size());
}

string Tools::DecryptPassword(const string& pwd)
{
   if (pwd.length() == 0)
      return "";
   uint8_t key[MD5_DIGEST_LENGTH];
   GenerateKey(key);
   vector<uint8_t> data = Base64::DecodeBin(pwd);
   vector<uint8_t> plain;
   AESDecrypt(key, data, plain);
   return string((const char *)&plain[0], plain.size());
}
#endif

string Tools::IPv4ToString(const uint32_t addr)
{
   ostringstream oss;
   oss << (addr >> 24) << "." << ((addr >> 16) & 0xFF) << "." << ((addr >> 8) & 0xFF) << "." << (addr & 0xFF);
   return oss.str();;
}

uint32_t Tools::StringToIPv4(const std::string& addr)
{
   int bytes[4];
   TokenIterator<int> byte(addr, ".");
   for (int i = 0; i < 4 && byte; ++i)
      bytes[i] = *byte++;
   return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

string Tools::PrefixToNetmask(const string& prefix)
{
   istringstream iss(prefix);
   uint32_t prefix_num;
   iss >> prefix_num;
   uint32_t hosts = (1 << (32 - prefix_num)) - 1;
   uint32_t mask = 0xFFFFFFFF ^ hosts;
   return IPv4ToString(mask);
}

string Tools::NetmaskToPrefix(const string& netmask)
{
   uint32_t mask = StringToIPv4(netmask);
   uint32_t bits = 0;
   for (int i = 0; i < 32 && ((mask >> (31 - i)) & 1); ++i)
      ++bits;
   ostringstream oss;
   oss << bits;
   return oss.str();
}

bool Tools::IsInSubnet(const string& host, const string& net, const string& prefix)
{
   uint32_t ip = Tools::StringToIPv4(host);
   uint32_t first = Tools::StringToIPv4(net);
   istringstream iss(prefix);
   uint32_t prefix_num;
   iss >> prefix_num;
   uint32_t hosts = 1 << (32 - prefix_num);
   uint32_t last = first + hosts - 1;
   return (ip >= first && ip <= last);
}

string Tools::hexdump(const uint8_t *buf, int len, bool upper, bool format, bool space)
{
   ostringstream oss;
   if (upper)
      oss << uppercase;
   oss << hex;
   for (int i = 0; i < len; ++i)
   {
      if (i > 0 && i % 16 == 0 && format)
         oss << endl;
      else if (i > 0 && space)
         oss << " ";
      oss << setfill('0') << setw(2) << (int)buf[i];
   }
   return oss.str();
}

void Tools::hex2bin(const string& hexdata, uint8_t *bin)
{
   unsigned int dlen = (unsigned int)hexdata.length() / 2;
   for (unsigned int i = 0; i < dlen; ++i)
   {
      string byte = hexdata.substr(i*2, 2);
      istringstream iss(byte);
      uint32_t value;
      iss >> hex >> value;
      bin[i] = (uint8_t)value;
   }
   return;
}

string Tools::Trim(const string& item)
{
   string::size_type first, last;
   first = item.find_first_not_of(' ');
   last = item.find_last_not_of(' ');
   if (first != string::npos)
      return item.substr(first, last - first + 1);
   return string("");
}

string Tools::Basename(const string& path)
{
   string::size_type pos = path.rfind('/');
   if (pos == string::npos)
      pos = path.rfind('\\');
   if (pos != string::npos)
      return path.substr(pos+1);
   return path;
}

#ifdef MINGW
string Tools::Unicode2Ascii(uint8_t *data, int len)
{
   string rc;
   for (int i = 0; i < len; ++i)
   {
      rc += *data & 0x7F;
      data += 2;
   }
   return rc;
}
#endif

string Tools::Base64::Encode(uint8_t const* bytes, unsigned int len)
{
   string ret;
   int i = 0;
   int j = 0;
   uint8_t char_array_3[3];
   uint8_t char_array_4[4];

   while (len--)
   {
      char_array_3[i++] = *(bytes++);
      if (i == 3)
      {
         char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
         char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
         char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
         char_array_4[3] = char_array_3[2] & 0x3f;
         for(i = 0; (i <4) ; i++)
            ret += base64_chars[char_array_4[i]];
         i = 0;
      }
   }
   if (i)
   {
      for (j = i; j < 3; j++)
         char_array_3[j] = '\0';
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;
      for (j = 0; (j < i + 1); j++)
         ret += base64_chars[char_array_4[j]];
      while ((i++ < 3))
         ret += '=';
   }
   return ret;
}

string Tools::Base64::Encode(const string& str)
{
   vector<uint8_t> data(str.begin(), str.end());
   return Encode(&data[0], data.size());
}

vector<uint8_t> Tools::Base64::DecodeBin(const string& str)
{
   int len = str.length();
   int i = 0;
   int j = 0;
   int in_ = 0;
   uint8_t char_array_4[4], char_array_3[3];
   vector<uint8_t> ret;

   while (len-- && ( str[in_] != '=') && is_base64(str[in_]))
   {
      char_array_4[i++] = str[in_]; in_++;
      if (i ==4)
      {
         for (i = 0; i <4; i++)
            char_array_4[i] = base64_chars.find(char_array_4[i]);
         char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
         char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
         char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
         for (i = 0; (i < 3); i++)
            ret.push_back(char_array_3[i]);
         i = 0;
      }
   }
   if (i) {
     for (j = i; j <4; j++)
        char_array_4[j] = 0;
     for (j = 0; j <4; j++)
        char_array_4[j] = base64_chars.find(char_array_4[j]);
     char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
     char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
     char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
     for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
   }
   return ret;
}

string Tools::Base64::Decode(const string& str)
{
   vector<uint8_t> data = DecodeBin(str);
   return string(data.begin(), data.end());
}

void Tools::Sleepms(unsigned long ms)
{
   unsigned long us = ms * 1000L;
   for (unsigned long i = 0; i < us / 999999; ++i)
      usleep(999999);
   usleep(us % 999999);
}
