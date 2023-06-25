#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iomanip>
#include "TokenIterator.h"
#include "tools.h"
#include "http_srv.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"

using namespace std;

const HttpSrv::HttpResponse::mime_t HttpSrv::HttpResponse::_mime_types[] =
{
   { "7z", "application/x-7z-compressed" },
   { "a", "application/octet-stream" },
   { "ac", "text/plain" },
   { "aif", "audio/aiff" },
   { "aiff", "audio/aiff" },
   { "ani", "application/x-navi-animation" },
   { "arc", "application/octet-stream" },
   { "art", "image/x-jg" },
   { "asf", "video/x-ms-asf" },
   { "asm", "text/x-asm" },
   { "asp", "text/asp" },
   { "asx", "application/x-mplayer2" },
   { "au", "audio/basic" },
   { "avi", "video/avi" },
   { "bin", "application/octet-stream" },
   { "bmp", "image/bmp" },
   { "bz2", "application/x-bzip2" },
   { "c", "text/plain" },
   { "c++", "text/plain" },
   { "cc", "text/plain" },
   { "cer", "text/plain" },
   { "cfg", "text/plain" },
   { "class", "application/java-byte-code" },
   { "com", "application/octet-stream" },
   { "conf", "text/plain" },
   { "cpio", "application/x-cpio" },
   { "cpp", "text/plain" },
   { "crl", "application/pkix-crl" },
   { "crt", "application/pkix-cert" },
   { "csh", "text/x-script.csh" },
   { "csv", "text/plain" },
   { "css", "text/css" },
   { "cxx", "text/plain" },
   { "db", "application/octet-stream" },
   { "def", "text/plain" },
   { "der", "application/x-x509-ca-cert" },
   { "diff", "text/plain" },
   { "dll", "application/octet-stream" },
   { "dmg", "application/octet-stream" },
   { "dmp", "application/octet-stream" },
   { "doc", "application/msword" },
   { "docx", "application/msword" },
   { "dot", "application/msword" },
   { "dump", "application/octet-stream" },
   { "dwg", "application/acad" },
   { "eps", "application/postscript" },
   { "exe", "application/octet-stream" },
   { "fli", "video/fli" },
   { "g3", "image/g3fax" },
   { "gif", "image/gif" },
   { "gpg", "application/pgp-encrypted" },
   { "gtar", "application/x-gtar" },
   { "gz", "application/x-gzip" },
   { "gzip", "application/x-gzip" },
   { "h", "text/plain" },
   { "help", "application/x-helpfile" },
   { "hlp", "application/hlp" },
   { "hqx", "application/binhex" },
   { "htm", "text/html" },
   { "html", "text/html" },
   { "ico", "image/x-icon" },
   { "img", "application/octet-stream" },
   { "image", "application/octet-stream" },
   { "in", "text/plain" },
   { "inf", "application/inf" },
   { "iso", "application/octet-stream" },
   { "java", "text/plain" },
   { "jpeg", "image/jpeg" },
   { "jpg", "image/jpeg" },
   { "js", "application/javascript" },
   { "key", "text/plain" },
   { "lha", "application/lha" },
   { "list", "text/plain" },
   { "log", "text/plain" },
   { "lst", "text/plain" },
   { "lzh", "application/octet-stream" },
   { "m", "text/plain" },
   { "mm", "text/plain" },
   { "m1v", "video/mpeg" },
   { "m2a", "audio/mpeg" },
   { "m2v", "video/mpeg" },
   { "m3u", "audio/x-mpequrl" },
   { "m4", "text/plain" },
   { "mht", "message/rfc822" },
   { "mhtml", "message/rfc822" },
   { "mid", "audio/midi" },
   { "midi", "audio/midi" },
   { "mime", "message/rfc822" },
   { "mjpg", "video/x-motion-jpeg" },
   { "mka", "audio/x-matroska" },
   { "mkv", "video/x-matroska" },
   { "mov", "video/quicktime" },
   { "mp2", "audio/mpeg" },
   { "mp3", "audio/mpeg3" },
   { "mpa", "audio/mpeg" },
   { "mpeg", "video/mpeg" },
   { "mpg", "video/mpeg" },
   { "mpga", "audio/mpeg" },
   { "mpp", "application/vnd.ms-project" },
   { "mpt", "application/x-project" },
   { "mpv", "application/x-project" },
   { "msi", "application/octet-stream" },
   { "o", "application/octet-stream" },
   { "p12", "application/pkcs-12" },
   { "p7b", "application/x-pkcs7-signature" },
   { "pac", "application/octet-stream" },
   { "pbm", "image/x-portable-bitmap" },
   { "pcap", "application/octet-stream" },
   { "pcapng", "application/octet-stream" },
   { "pcx", "image/x-pcx" },
   { "pdf", "application/pdf" },
   { "pem", "text/plain" },
   { "pfx", "application/pkcs-12" },
   { "pic", "image/pict" },
   { "pkg", "application/x-newton-compatible-pkg" },
   { "pl", "text/plain" },
   { "pm", "image/x-xpixmap" },
   { "png", "image/png" },
   { "pot", "application/mspowerpoint" },
   { "ppm", "image/x-portable-pixmap" },
   { "ppt", "application/mspowerpoint" },
   { "pptx", "application/mspowerpoint" },
   { "ps", "application/postscript" },
   { "pvk", "text/plain" },
   { "py", "text/x-script.phyton" },
   { "pyc", "application/x-bytecode.python" },
   { "qcow2", "application/octet-stream" },
   { "qif", "image/x-quicktime" },
   { "qt", "video/quicktime" },
   { "qtc", "video/x-qtc" },
   { "ra", "audio/x-pn-realaudio" },
   { "ram", "audio/x-pn-realaudio" },
   { "rar", "application/x-rar-compressed" },
   { "raw", "application/octet-stream" },
   { "rc", "text/plain" },
   { "rgb", "image/x-rgb" },
   { "rng", "application/ringing-tones" },
   { "rpm", "application/octet-stream" },
   { "rtf", "text/richtext" },
   { "s", "text/x-asm" },
   { "s3m", "audio/s3m" },
   { "sgml", "text/sgml" },
   { "sh", "text/x-script.sh" },
   { "shar", "application/x-shar" },
   { "shtml", "text/html" },
   { "sit", "application/x-sit" },
   { "snd", "audio/basic" },
   { "swf", "application/x-shockwave-flash" },
   { "tar", "application/x-tar" },
   { "tcl", "text/x-script.tcl" },
   { "tex", "application/x-tex" },
   { "texi", "application/x-texinfo" },
   { "texinfo", "application/x-texinfo" },
   { "text", "text/plain" },
   { "tgz", "application/gnutar" },
   { "tif", "image/tiff" },
   { "tiff", "image/tiff" },
   { "txt", "text/plain" },
   { "uu", "text/x-uuencode" },
   { "uue", "text/x-uuencode" },
   { "vbs", "text/vbscript" },
   { "vcs", "text/x-vcalendar" },
   { "vrml", "application/x-vrml" },
   { "vsd", "application/x-visio" },
   { "vst", "application/x-visio" },
   { "vsw", "application/x-visio" },
   { "wav", "audio/wav" },
   { "wmf", "windows/metafile" },
   { "wri", "application/mswrite" },
   { "xbm", "image/xbm" },
   { "xlsm", "application/excel" },
   { "xlsx", "application/excel" },
   { "xlt", "application/excel" },
   { "xlw", "application/excel" },
   { "xml", "text/xml" },
   { "xpm", "image/xpm" },
   { "z", "application/x-compress" },
   { "zip", "application/zip" },
   { "zsh", "text/x-script.zsh" }
};

HttpSrv::HttpSrv(const string& port, const string& ip) : PThread("_httpsrv"), _last_errno(0), _connection_timeout(120)
{
   _source_ip = ip;
   _port = port;
   SetRootDir("");
}

HttpSrv::~HttpSrv()
{
   End();
}

void HttpSrv::Run()
{
#ifdef SOCK_DEBUG
   Tools::debug << "\n### httpd thread started" << endl;
#endif
   _last_errno = 0;
   _srv.SetTimeOut(1);
   if (!_srv.StartServer(_port, _source_ip))
   {
      _last_errno = __lasterror;
      SetReturnCode(2);
      return;
   }
   _page_handlers.clear();
   Setup();
   PlugLockable("handlers", &_page_handlers);
   unsigned int count = 0;
   while (true)
   {
      SrvSock::AcceptedSock *cln_conn = _srv.ClientAccept();
      if (cln_conn)
      {
         ostringstream oss;
         oss << "task" << setfill('0') << setw(3) << ++count;
         HttpResponse *task = new HttpResponse(this, oss.str());
         _clients.push_back(task);
         task->PlugLockable("handlers", &_page_handlers);
         task->SetTimeOut(_connection_timeout);
         task->Start(cln_conn);
      }
      else if (_srv.GetErrorNumber() != __ETIMEDOUT)
      {
#ifdef SOCK_DEBUG
         Tools::debug << "### accept returned error: " << _srv.GetErrorMessage() << endl;
#endif
      }
      if (_clients.size() > 20)
         _clients.remove_if(HttpResponse::Terminated);
   }
   SetReturnCode(1);
}

void HttpSrv::OnExit()
{
#ifdef SOCK_DEBUG
   Tools::debug << "\n### httpd thread exiting" << endl;
#endif
   _srv.StopServer();
   for (list<HttpResponse *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
      delete (*it);
   _clients.clear();
}

void HttpSrv::SetRootDir(const string& path)
{
   char buf[1024];
   getcwd(buf, sizeof(buf));
   string cwd(buf);
   if (path.length() == 0)
      _root_dir = cwd;
   else if (path.at(0) == '/' || path.at(0) == '\\' || (path.length() > 2 && path.at(1) == ':'))
      _root_dir = path;
   else
   {
      chdir(path.c_str());
      getcwd(buf, sizeof(buf));
      _root_dir = buf;
      chdir(cwd.c_str());
   }
#ifdef MINGW
   if (_root_dir.length() > 0 && _root_dir.at(_root_dir.length()-1) != '\\')
      _root_dir += "\\";
#else
   if (_root_dir.length() > 0 && _root_dir.at(_root_dir.length()-1) != '/')
      _root_dir += "/";
#endif
}

void HttpSrv::SetServerAddress(const string& addr)
{
   if (!IsRunning())
      _source_ip = addr;
}

void HttpSrv::SetServerPort(const string& port)
{
   if (!IsRunning())
      _port = port;
}

int HttpSrv::GetErrorNumber()
{
   return _last_errno;
}

string HttpSrv::GetErrorMessage()
{
   string errmsg;
#ifdef MINGW
   char msg[1024];
   LPTSTR errTxt = 0;
   if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                     0, _last_errno, 0, (LPTSTR)&errTxt, sizeof(msg) - 1, 0))
   {
#ifdef UNICODE
         errmsg = Tools::Unicode2Ascii((uint8_t *)errTxt, wcslen(errTxt));
#else
         errmsg = errTxt;
#endif
         LocalFree(errTxt);
         if (errmsg.at(errmsg.length()-1) == '\n')
            errmsg = errmsg.substr(0, errmsg.length()-1);
   }
   else
   {
      errmsg = "No Error Description";
   }
#else
   errmsg = strerror(_last_errno);
#endif
   return errmsg;
}

void HttpSrv::AddText(const string& page, page_handler_t proc, bool keepalive)
{
   Lock(true, "handlers");
   handler_t::iterator it = _page_handlers.find(page);
   if (it == _page_handlers.end())
   {
      page_info_t pinfo;
      pinfo.proc = proc;
      pinfo.keepalive = keepalive;
      _page_handlers[page] = pinfo;
   }
   Unlock("handlers");
}

void HttpSrv::AddBin(const string& page, page_handler_t proc, bool keepalive)
{
   Lock(true, "handlers");
   handler_t::iterator it = _page_handlers.find(page);
   if (it == _page_handlers.end())
   {
      page_info_t pinfo;
      pinfo.type = TypeBin;
      pinfo.proc = proc;
      pinfo.keepalive = keepalive;
      _page_handlers[page] = pinfo;
   }
   Unlock("handlers");
}

void HttpSrv::AddFile(const string& page, const string& filename, bool chunked, bool keepalive)
{
   Lock(true, "handlers");
   handler_t::iterator it = _page_handlers.find(page);
   if (it == _page_handlers.end())
   {
      page_info_t pinfo;
      pinfo.type = TypeFile;
      pinfo.filename = _root_dir + filename;
      pinfo.chunked = chunked;
      pinfo.keepalive = keepalive;
      _page_handlers[page] = pinfo;
   }
   Unlock("handlers");
}

void HttpSrv::AddDir(const string& dir, const string& path, bool chunked, bool keepalive)
{
   Lock(true, "handlers");
   handler_t::iterator it = _page_handlers.find(dir);
   if (it == _page_handlers.end())
   {
      page_info_t pinfo;
      pinfo.type = TypeDir;
      pinfo.filename = _root_dir + path;
      pinfo.chunked = chunked;
      pinfo.keepalive = keepalive;
      _page_handlers[dir] = pinfo;
   }
   Unlock("handlers");
}

void HttpSrv::ReplaceVar(string& response, const string& var, const string& value)
{
   string::size_type pos;
   unsigned int var_len = var.length() + 2;

   while ((pos = response.find("$"+var+"$")) != string::npos)
         response.replace(pos, var_len, value);
}

void HttpSrv::ReplaceVar(string& response, const string& var, const int value)
{
   string::size_type pos;
   ostringstream oss;
   oss << value;
   unsigned int var_len = var.length() + 2;

   while ((pos = response.find("$"+var+"$")) != string::npos)
         response.replace(pos, var_len, oss.str());
}

void HttpSrv::GetAuthentication(const header_t& header, string& username, string& password)
{
   string::size_type basic_pos;
   header_t::const_iterator it = header.find("authorization");
   if (it != header.end() && (basic_pos = it->second.find("Basic ")) != string::npos)
   {
      string auth = Tools::Base64::Decode(it->second.substr(basic_pos + 6));
      string::size_type colon = auth.find(':');
      if (colon != string::npos)
      {
         username = auth.substr(0, colon);
         password = auth.substr(colon + 1);
      }
   }
}

void HttpSrv::AddAuthenticationRequest(response_t& response, const std::string& realm)
{
   response.header.push_back(header_config_t(RH_REPLACE, "WWW-Authenticate", "Basic realm=\"" + realm + "\""));
   response.status_code = HTTP_AUTH;
}

bool HttpSrv::GetRequestCookies(const header_t& header, args_t& cookies)
{
   header_t::const_iterator it = header.find("cookie");
   if (it != header.end() && it->second.length() > 0)
   {
      TokenIterator<string> attr(it->second, ",; ");
      while (attr)
      {
         string pair(*attr++);
         string::size_type separator = pair.find('=');
         if (separator != string::npos)
            cookies[Tools::Trim(pair.substr(0, separator))] = Tools::Trim(pair.substr(separator + 1));
      }
      return true;
   }
   return false;
}

#ifdef HAVE_SSL
bool HttpSrv::GetSSL()
{
   return _srv.GetSSL();
}

bool HttpSrv::SetSSL(bool ssl, bool use_cert, BaseSock::SSLMethod method, const string& ca_file)
{
   return _srv.SetSSL(ssl, use_cert, method, ca_file);
}

bool HttpSrv::UseCertificate(const string& cert_file, const string& pk_file, const string& pwd)
{
   return _srv.UseCertificate(cert_file, pk_file, pwd);
}
#endif

HttpSrv::HttpResponse::HttpResponse(HttpSrv *parent, const string& name) : PThread(name), _parent(parent), _connection_timeout(120)
{
}

HttpSrv::HttpResponse::~HttpResponse()
{
   End();
}

void HttpSrv::HttpResponse::Run()
{
   _conn = (SrvSock::AcceptedSock *)GetData();
#ifdef SOCK_DEBUG
   Tools::debug << "\n### thread " << _name << " started, connection from host " << _conn->GetHost() << endl;
#endif
   _conn->SetTimeOut(_connection_timeout);
   HttpSrv::HttpResponse::RequestStatus status;
   while ((status = ReadRequest()) != REQUEST_ERROR)
   {
      if (status == REQUEST_DONE_CLOSE || (status == REQUEST_OK && !ProcessRequest()))
         break;
   }
   usleep(10000);
   _conn->CloseConnection();
}

void HttpSrv::HttpResponse::OnExit()
{
#ifdef SOCK_DEBUG
   Tools::debug << "\n### thread " << _name << " terminated" << endl;
#endif
   delete _conn;
}

bool HttpSrv::HttpResponse::Terminated(const HttpSrv::HttpResponse *cTask)
{
   HttpResponse *task = const_cast<HttpResponse *>(cTask);
   if (!task->IsRunning())
   {
      delete task;
      return true;
   }
   return false;
}

void HttpSrv::HttpResponse::CreateHeader()
{
   _response_header.clear();
   _response_status = "HTTP/1.1 200 OK";
   time_t secs = time(0);
   StartCriticalSection();
   struct tm *tv = gmtime(&secs);
   string date = AscTime(tv);
   EndCriticalSection();
   //_response_header.push_back(response_header_entry_t("Server", "HttpSrv"));
   _response_header.push_back(response_header_entry_t("Date", date));
   _response_header.push_back(response_header_entry_t("Connection", "keep-alive"));
   _response_header.push_back(response_header_entry_t("Content-Type", "text/html"));
   _response_header.push_back(response_header_entry_t("Content-Length", "0"));
}

void HttpSrv::HttpResponse::UpdateHeader(header_mod_t& header)
{
   header_mod_t::iterator h;
   for (h = header.begin(); h != header.end(); ++h)
   {
      if (h->action == RH_ADD)
         AddHeaderEntry(h->key, h->value);
      else if (h->action == RH_REMOVE)
         RemoveHeaderEntry(h->key);
      else if (h->action == RH_REPLACE)
         ReplaceHeaderEntry(h->key, h->value);
   }
}

void HttpSrv::HttpResponse::AddHeaderEntry(const string& key, const string& value)
{
   _response_header.push_back(response_header_entry_t(key, value));
}

void HttpSrv::HttpResponse::RemoveHeaderEntry(const string& key)
{
   vector<response_header_entry_t>::iterator it;
   for (it = _response_header.begin(); it != _response_header.end(); ++it)
   {
      if (it->key == key)
      {
         _response_header.erase(it);
         break;
      }
   }
}

void HttpSrv::HttpResponse::ReplaceHeaderEntry(const string& key, const string& value)
{
   vector<response_header_entry_t>::iterator it;
   for (it = _response_header.begin(); it != _response_header.end(); ++it)
   {
      if (it->key == key)
      {
         it->value = value;
         break;
      }
   }
   if (it == _response_header.end())
      AddHeaderEntry(key, value);
}

bool HttpSrv::HttpResponse::GetHeaderEntry(const string& key, string *value)
{
   vector<response_header_entry_t>::iterator it;
   for (it = _response_header.begin(); it != _response_header.end(); ++it)
   {
      if (it->key == key)
      {
         if (value)
            *value = it->value;
         return true;
      }
   }
   return false;
}

bool HttpSrv::HttpResponse::GetRequestHeader(const string& key, string *value, bool lowercase)
{
   string lkey = key;
   transform(lkey.begin(), lkey.end(), lkey.begin(), ::tolower);
   header_t::iterator it = _request_header.find(lkey);
   if (it != _request_header.end())
   {
      if (value)
      {
         *value = it->second;
         if (lowercase)
            transform(value->begin(), value->end(), value->begin(), ::tolower);
      }
      return true;
   }
   return false;
}

void HttpSrv::HttpResponse::WriteHeader()
{
   vector<response_header_entry_t>::iterator it;
   _conn->WriteLine(_response_status);
   for (it = _response_header.begin(); it != _response_header.end(); ++it)
      _conn->WriteLine(it->key + ": " + it->value);
   _conn->WriteLine("");
#ifdef SOCK_DEBUG
   Tools::debug << "=> " << _response_status;
   for (it = _response_header.begin(); it != _response_header.end(); ++it)
      Tools::debug << "\n=> " << it->key + ": " + it->value;
   Tools::debug << "\n " << endl;
#endif
}

void HttpSrv::HttpResponse::WriteString(const string& response, bool keepalive)
{
   ostringstream oss;
   oss << response.length();
   ReplaceHeaderEntry("Content-Length", oss.str());
   if (!keepalive)
      ReplaceHeaderEntry("Connection", "close");
   WriteHeader();
   if (response.length() > 0 && _request_method != "HEAD")
      _conn->WriteData((const uint8_t *)response.c_str(), response.length());
}

void HttpSrv::HttpResponse::WriteBinaryHeader(long long total_len, bool chunked, long long offset)
{
   if (offset > 0)
   {
      _response_status = "HTTP/1.1 206 Partial Content";
      ostringstream range;
      range << "bytes=" << offset << "-/" << total_len;
      AddHeaderEntry("Content-Range", range.str());
   }
   if (chunked)
   {
      RemoveHeaderEntry("Content-Length");
      AddHeaderEntry("Transfer-Encoding", "chunked");
      WriteHeader();
   }
   else
   {
      ostringstream oss;
      oss << total_len - offset;
      ReplaceHeaderEntry("Content-Length", oss.str());
      WriteHeader();
   }
}

void HttpSrv::HttpResponse::WriteBinary(const uint8_t *data, int len, bool chunked, bool write_header)
{
   if (write_header)
      WriteBinaryHeader(len, chunked);
   if (data == 0 || len <= 0)
      return;
   if (chunked)
   {
      ostringstream oss;
      oss << setfill('0') << setw(4) << hex << len;
      _conn->WriteLine(oss.str());
      _conn->WriteData(data, len);
      _conn->WriteLine("");
   }
   else
   {
      _conn->WriteData(data, len);
   }
}

long long HttpSrv::HttpResponse::GetFileOffset()
{
   long long offset = 0;
   
   string range;
   GetRequestHeader("range", &range, true);
   if (range.substr(0, 6) == "bytes=" && range.at(range.length()-1) == '-')
   {
      string::size_type p1 = range.find_first_of('=');
      string::size_type p2 = range.find_last_of('-');
      istringstream offs(range.substr(p1 + 1, p2 - p1 - 1));
      offs >> offset;
   }
   return offset;
}

bool HttpSrv::HttpResponse::WriteFile(const string& filename, bool chunked, bool keepalive)
{
#if defined(MIPSEL) || defined(MINGW)
   struct stat64 attrib;
   if (stat64(filename.c_str(), &attrib) == -1)
      return false;
#else
   struct stat attrib;
   if (stat(filename.c_str(), &attrib) == -1)
      return false;
#endif
   StartCriticalSection();
   struct tm *tv = gmtime(&(attrib.st_mtime));
   string date = AscTime(tv);
   EndCriticalSection();
   string cached_date;
   if (GetRequestHeader("If-Modified-Since", &cached_date) && cached_date == date)
   {
      _response_status = "HTTP/1.1 304 Not Modified";
      WriteString("", keepalive);
      return true;
   }
   ifstream ifs(filename.c_str(), ios::in | ios::binary);
   if (!ifs)
      return false;
   AddHeaderEntry("Last-Modified", date);
   if (!keepalive)
      ReplaceHeaderEntry("Connection", "close");
   ifs.seekg(0, ios::end);
   long long file_len = (long long)ifs.tellg();
   ifs.seekg(0, ios::beg);
   if (file_len > (10 << 20))
   {
      ReplaceHeaderEntry("Content-Type", "application/octet-stream");
      AddHeaderEntry("Accept-Ranges", "bytes");
   }
   if (_request_method == "HEAD")
   {
      WriteBinaryHeader(file_len, false);
      return true;
   }
   long long offset = GetFileOffset();
   if (offset > 0)
      ifs.seekg(offset, ios::beg);
   WriteBinaryHeader(file_len, chunked, offset);
   uint8_t chunk[4096];
   long long to_read = file_len - offset, chunk_size;
   while (to_read > 0)
   {
      chunk_size = (to_read > (long long)sizeof(chunk)) ? (long long)sizeof(chunk) : to_read;
      ifs.read((char *)chunk, (streamsize)chunk_size);
      WriteBinary(chunk, (int)chunk_size, chunked, false);
      to_read -= chunk_size;
   }
   if (chunked)
      _conn->WriteLine("0\r\n");
   return true;
}

bool HttpSrv::HttpResponse::WriteError(status_code_t err, const string& errmsg, bool keepalive)
{
   string msg;
   if (err == HTTP_NOT_FOUND)
      msg = "Not Found";
   else if (err == HTTP_BAD_REQUEST)
      msg = "Bad request";
   else if (err == HTTP_REDIRECT_PERM)
      msg = "Moved Permanently";
   else if (err == HTTP_REDIRECT_TEMP)
      msg = "Moved Temporarly";
   else if (err == HTTP_AUTH)
      msg = "Unauthorized";
   else if (err == HTTP_FORBIDDEN)
      msg = "Forbidden";
   else if (err == HTTP_NOT_ALLOWED)
      msg = "Method Not Allowed";
   else if (err == HTTP_TIMEOUT)
      msg = "Request Timeout";
   else if (err == HTTP_LEN_REQUIRED)
      msg = "Length Required";
   else if (err == HTTP_NOT_IMPLEMENTED)
      msg = "Not Implemented";
   else if (err == HTTP_SERVICE_DOWN)
      msg = "Service Unavailable";
   else
   {
      err = HTTP_SERVER_ERROR;
      msg = "Internal Server Error";
   }
   ostringstream status;
   status << "HTTP/1.1 " << err << " " << msg;
   _response_status = status.str();
   string response;
   ReplaceHeaderEntry("Content-Type", "text/html");
   response = "<html><head><title>$N$ $M$</title></head><body><h2>$E$</h2></body></html>";
   ostringstream err_code;
   err_code << err;
   ReplaceVar(response, "N", err_code.str());
   ReplaceVar(response, "M", msg);
   ReplaceVar(response, "E", (errmsg.length() > 0) ? errmsg : msg);
   WriteString(response, keepalive);
   return false;
}

string HttpSrv::HttpResponse::AscTime(struct tm *tv)
{
   const char *mon[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                          "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
   const char *wday[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
   ostringstream oss;
   oss.fill('0');
   oss << wday[tv->tm_wday] << ", " << setw(2) << tv->tm_mday << " " << mon[tv->tm_mon] << " ";
   oss << tv->tm_year+1900 << " " << setw(2) << tv->tm_hour << ":" << setw(2) << tv->tm_min;
   oss << ":" << setw(2) << tv->tm_sec << " GMT";
   return oss.str();
}

void HttpSrv::HttpResponse::SelectFileType(const string& filename)
{
   string::size_type pos = filename.rfind('.');
   if (pos != string::npos)
   {
      string extension = filename.substr(pos+1);
      transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
      for (unsigned int i = 0; i < sizeof(_mime_types) / sizeof(mime_t); ++i)
      {
         if (extension == _mime_types[i].suffix)
         {
            ReplaceHeaderEntry("Content-Type", _mime_types[i].type);
            break;
         }
      }
   }
}
bool HttpSrv::HttpResponse::HandlerExists()
{
   handler_t::iterator handler;
   handler_t *handlers = (handler_t *)Lock(false, "handlers");
   for (handler = handlers->begin(); handler != handlers->end(); ++handler)
   {
      if (handler->first == _request_page && (handler->second.type == TypeText || handler->second.type == TypeBin))
         break;
   }
   Unlock("handlers");
   return (handler != handlers->end());
}

bool HttpSrv::HttpResponse::FindHandler(handler_t::iterator& handler)
{
   handler_t *handlers = (handler_t *)Lock(false, "handlers");
   handler_t::iterator found = handlers->end();
   for (handler = handlers->begin(); handler != handlers->end(); ++handler)
   {
      if (handler->second.type == TypeDir && _request_page.length() > handler->first.length() &&
          handler->first == _request_page.substr(0, handler->first.length()))
         found = handler;
      else if (handler->first == _request_page && handler->second.type != TypeDir)
         break;
   }
   if (handler == handlers->end()) handler = found;
   Unlock("handlers");
   return (handler != handlers->end());
}

bool HttpSrv::HttpResponse::SkipLines(int *bytes)
{
   string line;
   do
   {
      if (!_conn->ReadLine(line))
         return false;
      *bytes += line.length() + 2;
   } while (line.length() > 0);
   return true;
}

string HttpSrv::HttpResponse::GetValue(const string& line, const string& label)
{
   string::size_type start_pos = line.find(label) + label.length() + 1;
   string::size_type end_pos = line.find('"', start_pos);
   if (end_pos == string::npos)
      return "";
   string value = line.substr(start_pos, end_pos - start_pos);
   if (label.find("filename") != string::npos)
   {
      string::size_type pos = value.rfind('/');
      if (pos == string::npos)
         pos = value.rfind('\\');
      if (pos != string::npos)
         value = value.substr(pos+1);
   }
   return value;
}

bool HttpSrv::HttpResponse::SaveUpload(const string& content_type, int content_len)
{
   string::size_type pos = content_type.find("boundary=");
   if (pos == string::npos)
      return false;
   string boundary = "--" + content_type.substr(pos+9);
   int to = _conn->SetTimeOut(4);
   string line;
   if (!_conn->ReadLine(line) || line != boundary)
      return false;
   int tot_bytes = line.length() + 2;
   while (line.find(" filename=") == string::npos)
   {
      if (line.find(" name=") != string::npos)
      {
         string field_name = GetValue(line, " name=");
         if (field_name.length() == 0 || !SkipLines(&tot_bytes))
            return false;
         if (!_conn->ReadLine(line))
            return false;
         tot_bytes += line.length() + 2;
         _request_args[field_name] = line;
      }
      if (!_conn->ReadLine(line))
         return false;
      tot_bytes += line.length() + 2;
   }
   string fname = GetValue(line, " filename=");
   if (fname.length() == 0 || !SkipLines(&tot_bytes))
      return false;
   char tmpname[] = "upload.XXXXXX";
#ifdef MINGW
   TCHAR tmpdir[MAX_PATH];
   GetTempPath(MAX_PATH, tmpdir);
#ifdef UNICODE
   string tmpfile = Tools::Unicode2Ascii((uint8_t *)tmpdir, wcslen(tmpdir)) + mktemp(tmpname);
#else
   string tmpfile = string(tmpdir) + mktemp(tmpname);
#endif
   ofstream ofs(tmpfile.c_str(), ios::out | ios::binary | ios::trunc);
   if (!ofs)
      return false;
#else
   int tmp_fd = mkstemp(tmpname);
   if (tmp_fd == -1)
      return false;
   string tmpfile(tmpname);
#endif
   uint8_t buffer[4096];
   bool rc = false;
   int bytes = _conn->ReadData(buffer, sizeof(buffer));
   while (bytes > 0)
   {
      tot_bytes += bytes;
      string buf((const char*)buffer, bytes);
      uint8_t peek[boundary.length() + 2];
      int peek_bytes = (tot_bytes < content_len) ? _conn->ReadData(peek, sizeof(peek), true) : 0;
      if (peek_bytes > 0)
         buf.append((const char *)peek, peek_bytes);
      if ((pos = buf.find("\r\n" + boundary)) != string::npos)
      {
#ifdef MINGW
         ofs.write(&buf[0], pos);
#else
         write(tmp_fd, &buf[0], pos);
#endif
         rc = true;
         break;
      }
#ifdef MINGW
      ofs.write((const char *)buffer, bytes);
#else
      write(tmp_fd, &buffer[0], bytes);
#endif
      bytes = _conn->ReadData(buffer, sizeof(buffer));
   }
#ifdef MINGW
   ofs.close();
#else
   close(tmp_fd);
#endif
   int to_read = content_len - tot_bytes, chunk_size;
   while (bytes > 0 && to_read > 0)
   {
      chunk_size = (to_read > (int)sizeof(buffer)) ? (int)sizeof(buffer) : to_read;
      bytes = _conn->ReadData(buffer, chunk_size);
      to_read -= bytes;
   }
   _request_header["x-httpsrv-tmpfile"] = tmpfile;
   _request_header["x-httpsrv-filename"] = fname;
   _conn->SetTimeOut(to);
   return rc;
}

HttpSrv::HttpResponse::RequestStatus HttpSrv::HttpResponse::ReadRequest()
{
   _request_header.clear();
   _request_args.clear();
   int line_count = 0;
   string line;
   if (!_conn->ReadLine(line) || line.length() < 5)
      return REQUEST_ERROR;
   string reqType = line.substr(0, 8);
   transform(reqType.begin(), reqType.end(), reqType.begin(), ::tolower);
   if (reqType.substr(0, 4) == "get ")
      _request_method = "GET";
   else if (reqType.substr(0, 5) == "head ")
      _request_method = "HEAD";
   else if (reqType.substr(0, 5) == "post ")
      _request_method = "POST";
   else if (reqType.substr(0, 8) == "options ")
      _request_method = "OPTIONS";
   else
   {
#ifdef SOCK_DEBUG
      Tools::debug << "\n<= " << line;
#endif
      return (RequestStatus)WriteError(HTTP_NOT_IMPLEMENTED);
   }
   string::size_type start_pos = _request_method.length() + 1;
   string::size_type end_pos = line.find(" HTTP/1.");
   if (end_pos == string::npos)
      end_pos = line.find(" http/1.");
   _request_page = Tools::Trim(line.substr(start_pos, end_pos - start_pos));
   if (_request_page.length() == 0)
      return REQUEST_ERROR;
   _request_page = Unescape(_request_page);
#ifdef SOCK_DEBUG
   Tools::debug << "\n<= " << line;
#endif
   while (_conn->ReadLine(line) && line.length() > 0)
   {
      ++line_count;
      if (line_count > _header_max_lines || line.length() > (size_t)_header_max_line_len)
         break;
#ifdef SOCK_DEBUG
      Tools::debug << "\n<= " << line;
#endif
      string::size_type pos = line.find(":");
      if (pos == string::npos)
         break;
      string key = Tools::Trim(line.substr(0, pos));
      transform(key.begin(), key.end(), key.begin(), ::tolower);
      header_t::iterator it = _request_header.find(key);
      if (it == _request_header.end())
         _request_header[key] = Tools::Trim(line.substr(pos+1));
      else
         _request_header[key] = it->second + "," + Tools::Trim(line.substr(pos+1));
   }
#ifdef SOCK_DEBUG
   Tools::debug << "\n " << endl;
#endif
   if (line.length() > 0)
      return REQUEST_ERROR;
   if (_request_method == "OPTIONS")
   {
      CreateHeader();
      RemoveHeaderEntry("Content-Type");
      ReplaceHeaderEntry("Connection", "close");
      AddHeaderEntry("Allow:", "OPTIONS, GET, HEAD, POST");
      AddHeaderEntry("Public:", "OPTIONS, GET, HEAD, POST");
      WriteHeader();
      return REQUEST_DONE_CLOSE;
   }
   if (_request_method == "POST")
   {
      string len_str;
      GetRequestHeader("content-length", &len_str);
      istringstream iss(len_str);
      int content_len;
      iss >> content_len;
      string content_type;
      GetRequestHeader("content-type", &content_type);
      if (content_type.find("multipart/form-data") != string::npos)
      {
         if (!SaveUpload(content_type, content_len))
            return (RequestStatus)WriteError(HTTP_BAD_REQUEST);
      }
      else
      {
         uint8_t *data = new uint8_t[content_len];
         if (_conn->ReadDataExact(data, content_len) != content_len)
         {
            delete[] data;
            return REQUEST_ERROR;
         }
#ifdef SOCK_DEBUG
         Tools::debug << "<= " << string((const char *)data, content_len) << "\n " << endl;
#endif
         if (content_type.find("application/x-www-form-urlencoded") != string::npos)
            ParseArguments(string((const char *)data, content_len));
         else
            _request_header["x-httpsrv-post-data"] = string((const char *)data, content_len);
         delete[] data;
      }
   }
   string::size_type args_pos;
   if ((args_pos = _request_page.find('?')) != string::npos)
   {
      ParseArguments(_request_page.substr(args_pos + 1));
      _request_page = _request_page.substr(0, args_pos);
   }
   string username, password;
   GetAuthentication(_request_header, username, password);
   _request_header["x-httpsrv-username"] = username;
   _request_header["x-httpsrv-password"] = password;
#ifdef HAVE_SSL
   _request_header["x-httpsrv-cert-cn"] = _conn->GetPeerCertificateCommonName();
   _request_header["x-httpsrv-cert-email"] = _conn->GetPeerCertificateEmail();
   _request_header["x-httpsrv-cert-expiry"] = _conn->GetPeerCertificateExpiry();
   _request_header["x-httpsrv-cert-issuer"] = _conn->GetPeerCertificateIssuer();
   _request_header["x-httpsrv-sni"] = _conn->GetSNI();
   _request_header["x-httpsrv-ssl-ver"] = _conn->GetSSLVersion();
#endif
   response_t response;
   _parent->RequestReceived(_conn->GetHost(), _request_method, _request_page, _request_args, _request_header, response);
   if (response.status_code != HTTP_OK)
   {
      CreateHeader();
      UpdateHeader(response.header);
      if (response.status_code == HTTP_OK_DONE)
      {
         WriteString(response.content, response.keepalive);
         return (response.keepalive) ? REQUEST_DONE_KEEP : REQUEST_DONE_CLOSE;
      }
      WriteError(response.status_code, response.content, response.keepalive);
      return (response.keepalive) ? REQUEST_DONE_KEEP : REQUEST_ERROR;
   }
#ifdef SOCK_DEBUG
   //Tools::debug << "### request = " << _request_method;
   //Tools::debug << "\n### page    = \"" << _request_page << "\"";
   //args_t::iterator it;
   //int n = 0;
   //for (it = _request_args.begin(); it != _request_args.end(); ++it, ++n)
   //   Tools::debug << "\n### args[" << n << "] = " << it->first << ":\"" << it->second << "\"";
   //Tools::debug << endl;
#endif
   return REQUEST_OK;
}

void HttpSrv::RequestReceived(const string& from, const string& method, string& page,
                              const args_t& args, header_t& header, response_t& response)
{
}

const string HttpSrv::HttpResponse::Unescape(const string& str)
{
   string ret(str);
   string::size_type pos;

   string content_type;
   if (_request_method == "POST" && GetRequestHeader("content-type", &content_type) &&
                                    content_type.find("application/x-www-form-urlencoded") != string::npos)
   {
      pos = ret.find('+');
      while (pos != string::npos)
      {
         ret.replace(pos, 1, 1, ' ');
         pos = ret.find('+', pos+1);
      }
   }
   pos = ret.find('%');
   while (pos != string::npos && ret.length() - pos > 2)
   {
      if (::isxdigit(ret[pos+1]) && ::isxdigit(ret[pos+2]))
      {
         int c;
         istringstream code(ret.substr(pos+1, 2));
         code >> hex >> c;
         ret.replace(pos, 3, 1, c);
      }
      pos = ret.find('%', pos+1);
   }
   return ret;
}

void HttpSrv::HttpResponse::ParseArguments(const string& parameters)
{
   TokenIterator<string> sections(parameters, "?");
   while (sections)
   {
      string section = (*sections++);
      TokenIterator<string> args(section, "&");
      while (args)
      {
         string arg = (*args++);
         TokenIterator<string> item(arg, "=");
         vector<string> items;
         while (item)
            items.push_back(Unescape((*item++)));
         if (items.size() == 2)
            _request_args[items[0]] = items[1];
         else
            _request_args[items[0]] = "";
      }
   }
}

bool HttpSrv::HttpResponse::ProcessRequest()
{
   handler_t::iterator it;
   response_t response;
   CreateHeader();
   if (!FindHandler(it))
      return WriteError(HTTP_NOT_FOUND);
   if (it->second.type == TypeText)
   {
      SelectFileType(it->first);
      (_parent->*it->second.proc)(_request_args, _request_header, response);
      UpdateHeader(response.header);
      if (response.status_code == HTTP_OK)
         WriteString(response.content, it->second.keepalive);
   }
   else if (it->second.type == TypeBin)
   {
      SelectFileType(it->first);
      (_parent->*it->second.proc)(_request_args, _request_header, response);
      UpdateHeader(response.header);
      if (!it->second.keepalive)
         ReplaceHeaderEntry("Connection", "close");
      if (response.status_code == HTTP_OK)
         WriteBinary(&response.data[0], response.data.size());
   }
   else if (it->second.type == TypeFile)
   {
      SelectFileType(it->second.filename);
      if (!WriteFile(it->second.filename, it->second.chunked, it->second.keepalive))
         return WriteError(HTTP_NOT_FOUND);
   }
   else if (it->second.type == TypeDir)
   {
      string filename = it->second.filename + _request_page.substr(it->first.length());
      SelectFileType(filename);
      if (!WriteFile(filename, it->second.chunked, it->second.keepalive))
         return WriteError(HTTP_NOT_FOUND);
   }
   if (response.status_code != HTTP_OK)
      return WriteError(response.status_code, response.content);
   return it->second.keepalive;
}

#pragma GCC diagnostic pop
