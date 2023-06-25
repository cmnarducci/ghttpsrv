#ifndef __HTTP_SRV_H__
#define __HTTP_SRV_H__

#include <string>
#include <list>
#include <map>
#include <vector>
#include "thread.h"
#include "srv_sock.h"

class HttpSrv : public PThread
{
public:
   HttpSrv(const std::string& port, const std::string& ip = "0.0.0.0");
   ~HttpSrv();
   void Run();
   void OnExit();
   void SetRootDir(const std::string& path);
   void SetServerAddress(const std::string& addr);
   void SetServerPort(const std::string& port);
   std::string GetRootDir() { return _root_dir; };
   std::string GetServerPort() { return _port; };
   void SetConnectionTimeOut(int to) { _connection_timeout = to; };
   int GetErrorNumber();
   std::string GetErrorMessage();
#ifdef HAVE_SSL
   bool GetSSL();
   bool SetSSL(bool ssl, bool use_cert = false, BaseSock::SSLMethod method = BaseSock::TLS, const std::string& ca_file = "");
   bool UseCertificate(const std::string& cert_file, const std::string& pk_file, const std::string& pwd = "");
#endif

protected:
   virtual void Setup() = 0;
   enum header_action { RH_ADD, RH_REMOVE, RH_REPLACE };
   struct header_config_t
   {
      header_config_t(header_action a, const std::string& k, const std::string& v = "") : action(a), key(k), value(v) {};
      header_action action;
      std::string key;
      std::string value;
   };
   enum status_code_t
   {
      HTTP_OK = 200,
      HTTP_REDIRECT_PERM = 301,
      HTTP_REDIRECT_TEMP = 302,
      HTTP_NOT_MODIFIED = 304,
      HTTP_BAD_REQUEST = 400,
      HTTP_AUTH = 401,
      HTTP_FORBIDDEN = 403,
      HTTP_NOT_FOUND = 404,
      HTTP_NOT_ALLOWED = 405,
      HTTP_TIMEOUT = 408,
      HTTP_LEN_REQUIRED = 411,
      HTTP_SERVER_ERROR = 500,
      HTTP_NOT_IMPLEMENTED = 501,
      HTTP_SERVICE_DOWN = 503,
      HTTP_OK_DONE = 1001
   };
   typedef std::vector<header_config_t> header_mod_t;
   struct response_t
   {
      response_t() { status_code = HTTP_OK; keepalive = false; };
      status_code_t status_code;
      bool keepalive;
      header_mod_t header;
      std::string content;
      std::vector<uint8_t> data;
   };
   typedef std::map<std::string,std::string> args_t;
   typedef std::map<std::string,std::string> header_t;
   enum page_type_t { TypeFile, TypeDir, TypeText, TypeBin };
   typedef void (HttpSrv::*page_handler_t)(args_t& args, header_t& header, response_t& response);
   struct page_info_t
   {
      page_info_t() { type = TypeText; proc = 0; chunked = false; keepalive = false; }; 
      page_type_t type;
      page_handler_t proc;
      std::string filename;
      bool chunked;
      bool keepalive;
   };
   typedef std::map<std::string, page_info_t> handler_t;
   void AddText(const std::string& page, page_handler_t proc, bool keepalive = true);
   void AddBin(const std::string& page, page_handler_t proc, bool keepalive = true);
   void AddFile(const std::string& page, const std::string& filename, bool chunked = false, bool keepalive = true);
   void AddDir(const std::string& dir, const std::string& path, bool chunked = false, bool keepalive = true);
   static void ReplaceVar(std::string& response, const std::string& var, const std::string& value);
   static void ReplaceVar(std::string& response, const std::string& var, const int value);
   static void GetAuthentication(const header_t& header, std::string& username, std::string& password);
   static void AddAuthenticationRequest(response_t& response, const std::string& realm);
   static bool GetRequestCookies(const header_t& header, args_t& cookies);
   virtual void RequestReceived(const std::string& from, const std::string& method, std::string& page,
                                const args_t& args, header_t& header, response_t& response);

private:
   class HttpResponse : public PThread
   {
   public:
      HttpResponse(HttpSrv *parent, const std::string& name = "http_request");
      virtual ~HttpResponse();
      static bool Terminated(const HttpResponse *cTask);
      void SetTimeOut(int to) { _connection_timeout = to; };

   protected:
      void Run();
      void OnExit();

   private:
      enum RequestStatus
      {
         REQUEST_ERROR = 0,
         REQUEST_OK,
         REQUEST_DONE_CLOSE,
         REQUEST_DONE_KEEP
      };
      void CreateHeader();
      void UpdateHeader(header_mod_t& header);
      void AddHeaderEntry(const std::string& key, const std::string& value);
      void RemoveHeaderEntry(const std::string& key);
      void ReplaceHeaderEntry(const std::string& key, const std::string& value);
      bool GetHeaderEntry(const std::string& key, std::string *value = 0);
      bool GetRequestHeader(const std::string& key, std::string *value = 0, bool lowercase = false);
      void WriteHeader();
      void WriteString(const std::string& response, bool keepalive = true);
      void WriteBinaryHeader(long long total_len, bool chunked = false, long long offset = 0);
      void WriteBinary(const uint8_t *data, int len, bool chunked = false, bool write_header = true);
      bool WriteFile(const std::string& filename, bool chunked, bool keepalive);
      bool WriteError(status_code_t err, const std::string& errmsg = "", bool keepalive = false);
      bool SaveUpload(const std::string& content_type, int content_len);
      bool SkipLines(int *bytes);
      std::string GetValue(const std::string& line, const std::string& label);
      std::string AscTime(struct tm *tv);
      void SelectFileType(const std::string& filename);
      bool FindHandler(handler_t::iterator& handler);
      bool HandlerExists();
      RequestStatus ReadRequest();
      const std::string Unescape(const std::string& str);
      void ParseArguments(const std::string& parameters);
      bool ProcessRequest();
      long long GetFileOffset();
      HttpSrv *_parent;
      SrvSock::AcceptedSock *_conn;
      std::string _request_method;
      std::string _request_page;
      header_t _request_header;
      args_t _request_args;
      struct response_header_entry_t
      {
         response_header_entry_t() {};
         response_header_entry_t(const std::string& k, const std::string& v) : key(k), value(v) {};
         std::string key;
         std::string value;
      };
      std::vector<response_header_entry_t> _response_header;
      std::string _response_status;
      int _connection_timeout;
      struct mime_t
      {
         const char *suffix;
         const char *type;
      };
      static const mime_t _mime_types[];
      static const int _header_max_line_len = 2048;
      static const int _header_max_lines = 20;
   };
   int _last_errno;
   int _connection_timeout;
   std::string _source_ip;
   std::string _port;
   SrvSock _srv;
   std::list<HttpResponse *> _clients;
   handler_t _page_handlers;
   std::string _root_dir;
};

#endif
