#ifndef __TESTSRV_H__
#define __TESTSRV_H__

#include <cstdarg>
#include <string>
#include <sstream>
#ifdef HAVE_DUO
#include <set>
#endif
#include "http_srv.h"

class wxTextCtrl;

class Server : public HttpSrv
{
public:
   Server();
   virtual ~Server();

   enum LogType
   {
      LOG_INFO = 0,
      LOG_ERROR,
      LOG_DEBUG,
   };
   void Log(LogType type, const char *fmt,...);
   void SetUserPassword(const std::string& user, const std::string& password);
   void SetLogCtrl(wxTextCtrl *log) { _logctrl = log; };

protected:
   void Setup();
   void AboutPage(args_t& args, header_t& header, response_t& response);
   void UserPage(args_t& args, header_t& header, response_t& response);
   void FormPage(args_t& args, header_t& header, response_t& response);
   void UploadPage(args_t& args, header_t& header, response_t& response);
   void BrowsePage(args_t& args, header_t& header, response_t& response);
   void RequestReceived(const std::string& from, const std::string& method, std::string& page,
                        const args_t& args, header_t& header, response_t& response);

private:
   struct entry_t
   {
      entry_t() {};
      entry_t(const std::string& n, const mode_t m, const long long s) : name(n), mode(m), size(s) {};
      std::string name;
      mode_t mode;
      long long size;
   };
   static std::string Now();
   void DirectoryEntry(std::ostringstream& oss, const entry_t& entry, const std::string& folder);
   std::string CreateAnchors(const std::string& path);
   std::string _auth_user;
   std::string _auth_pwd;
   bool _chunked;
   bool _keepalive;
#ifdef HAVE_DUO
   std::set<std::string> _duo_authenticated;
#endif
   wxTextCtrl *_logctrl;
   static pthread_mutex_t _log_mutex;
};

#endif
