#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <cstring>
#include <map>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#if defined(MACOS) && defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpotentially-evaluated-expression"
#endif
#include <wx/textctrl.h>
#if defined(MACOS) && defined(__clang__)
#pragma clang diagnostic pop
#endif
#include "server.h"

using namespace std;

pthread_mutex_t Server::_log_mutex = PTHREAD_MUTEX_INITIALIZER;

Server::Server() : HttpSrv("80"), _chunked(false), _keepalive(true)
{
}

Server::~Server()
{
}

void Server::Setup()
{
   AddText("/about", (page_handler_t)&Server::AboutPage, _keepalive);
   AddText("/user", (page_handler_t)&Server::UserPage, _keepalive);
   AddText("/send", (page_handler_t)&Server::FormPage, _keepalive);
   AddText("/upload", (page_handler_t)&Server::UploadPage, _keepalive);
   AddText("/browse", (page_handler_t)&Server::BrowsePage, _keepalive);
   AddDir("/", "", _chunked, _keepalive);
}

void Server::RequestReceived(const string& from, const string& method, string& page,
                             const args_t& /*args*/, header_t& header, response_t& response)
{
   if (page != "/browse")
      Log(LOG_INFO, "From %s: %s %s", from.c_str(), method.c_str(), page.c_str());
   if (page == "/favicon.ico")
   {
      response.status_code = HTTP_NOT_FOUND;
      return;
   }
   if (_auth_user.length() > 0)
   {
      if (header["x-httpsrv-username"] != _auth_user || header["x-httpsrv-password"] != _auth_pwd)
      {
         AddAuthenticationRequest(response, "HttpSrv");
         return;
      }
   }
   if (page != "/browse")
   {
      string rpage = page.substr(1);
      string path = GetRootDir() + rpage;
      int plen = path.length();
      if (!(plen == 3 && path[1] == ':') && plen > 1 && (path[plen-1] == '/' || path[plen-1] == '\\'))
         path = path.substr(0, plen-1);
      struct stat info;
      if (stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR))
      {
         response.header.push_back(header_config_t(RH_ADD, "Location", "/browse?path=" + rpage));
         response.status_code = HTTP_REDIRECT_TEMP;
         response.keepalive = true;
      }
   }
}

void Server::BrowsePage(args_t& args, header_t& /*header*/, response_t& response)
{
   struct stat info;
   DIR *dir = 0;
   args_t::const_iterator arg = args.find("path");
   if (arg == args.end() || arg->second.find("..") != string::npos)
   {
      response.status_code = HTTP_FORBIDDEN;
      return;
   }
   string path = GetRootDir() + arg->second;
   int plen = path.length();
   if (!(plen == 3 && path[1] == ':') && plen > 1 && (path[plen-1] == '/' || path[plen-1] == '\\'))
      path = path.substr(0, plen-1);
   if (stat(path.c_str(), &info) != 0 || (info.st_mode & S_IFDIR) == 0 || (dir = opendir(path.c_str())) == 0)
   {
      response.status_code = HTTP_FORBIDDEN;
      return;
   }
   string folder(arg->second);
   if (folder.length() > 0 && folder.at(folder.length()-1) == '\\')
      folder.at(folder.length()-1) = '/';
   else if (folder.length() > 0 && folder.at(folder.length()-1) != '/')
      folder += "/";
   ostringstream oss;
   string css =
"body { font-family: Century Gothic, sans-serif; }"
"div.title { font-size: 1.5em; border-radius: .5em; width: 96%; margin: 25px auto 0px auto; background: #DAE4F0;"
"            display: flex; justify-content: center; align-items: center; color: #7A8490; }"
"a { text-decoration: none; color: black;}"
"a:hover { color: #7A8490; }"
"a:visited { color: black; }"
"a:visited:hover { color: #7A8490; }"
"a.title:hover { color: black; }"
"a.title:visited { color: #7A8490; }"
"a.title:visited:hover { color: black; }"
"table { font-size: 0.9em; padding: 25px 25px; }"
"td.name { padding: 0px 50px 0px 0px; }"
"td.size { padding: 1px 0px 0px 0px; text-align: right; font-size: 0.8em; color: #7A8490; }"
"td.icon { height: 20px; width: 24px; background-position: center; background-repeat: no-repeat; }"
"td#file {"
" background-image: url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAAXNSR0IArs4c6QAAAO9JREFUOBGNUzEOAVE"
                                             "Q3RWNG5AoRUmvUSolouAqREJwEo1C4wQicQFCreUAIhER7xXD/P9n+ZO8zMybN283/++myTeqKEdA4Us51Q"
                                             "Ld2mG8hoKix0nbQ3ECOkJIzkmBzCdfVO+XExAtwDHRBv6C3zdAbIE50JZhrMEGCzvgAfCc+kAQq4DJJpYyi"
                                             "n0D0Qc5HzBJUgM3MHhSU4C38QnL4IApry0qLIMSNpsZ2zzMq55ZBi8Inlqkas6csAz4hOgbsQzqMOBdWzEG"
                                             "edQDy2APQVeLftX6O7hDyAP8F2UIbiJKpUCuAEMg63cWKZdnwJnEG0WHHuHWg8P8AAAAAElFTkSuQmCC);"
"}"
"td#folder {"
" background-image: url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAAXNSR0IArs4c6QAAALhJREFUOBFjYKAQMCL"
                                             "pNweyFZH434Ds7UD8G0kMg8kEFQFpzgXiP0hYAchuAGK8AOaCCKAqkOY1aKqfAPkn0MSQuQdABpgBcQwQ/w"
                                             "NifIqB0mDwE0iuh7JXgLxQBsSHgPgIECN7ARe7HKgODligLHSnwxVgYYC8CwewQIQLkMoYNYCBARQG/4GY1"
                                             "LAAqWcG6QUlpHQgVgPi00BMDAgHKroHxCBDLsOSsiWQIw3EpIDHQMUnSdGAVS0A4x4kIH02UikAAAAASUVO"
                                             "RK5CYII=);"
"}"
"td#unknown {"
" background-image: url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAAXNSR0IArs4c6QAAAUFJREFUOBGNkrsuBVE"
                                             "Uhmdc41KIQhBRSEQjIbQ4iUKiQkHlAYha40FUipOITonGKyDRKEQjEYTQuYS4fN+YkTPGmTl/8s1ea6+1L7"
                                             "P2CoOsOpiahqZs6HfmBesA3sN4ao1xEvS7YQtMqqYeArLRwGcYhmATPFV7G/I0Q7DPBDeYBxcswTUcQZEuS"
                                             "RgzqQ4G4QzWYQAuoEh3JDSa5AatkPxvF/Y9FOmNhCsYd4NH6AT1Cu2Rlf95jnPDeoxe8OnOwduMwgnkydfy"
                                             "oCmLuAs70AYGrMUe3IDxBfCgv/IZP12gLORIZAVBC+McLMIX2COzUIZKPeEcVk5U2is4q/GEh+xDc+ynhv+"
                                             "uZsIx+Cun8AD9kFQeszbZ0p6slsFaZOQzVtMtgaQ/vIFFzShvA5M/wOYqQS0tTlpaE7hliPo+HfrxvgHBhT"
                                             "H36MERLwAAAABJRU5ErkJggg==);"
"}";
   string title = (arg->second.length() > 0) ? "ROOT:" + arg->second : "ROOT";
   oss << "<html><head><title>" << title << "</title><style>" << css << "</style></head><body>";
   oss << "<div class=\"title\"><p>" << CreateAnchors(title) << "</p></div><table>";
   struct dirent *result;
   map<string,entry_t> content;
   StartCriticalSection();
   result = readdir(dir);
   while (result)
   {
      string name(result->d_name);
      EndCriticalSection();
      string path = GetRootDir() + folder + name;
#if defined(MIPSEL) || defined(MINGW)
      struct stat64 info;
      if (name.length() > 0 && name.at(0) != '.' && stat64(path.c_str(), &info) == 0)
#else
      struct stat info;
      if (name.length() > 0 && name.at(0) != '.' && stat(path.c_str(), &info) == 0)
#endif
      {
         string key(name);
         transform(key.begin(), key.end(), key.begin(), ::tolower);
         key = ((info.st_mode & S_IFDIR) ? "01" : "02") + key;
         content[key] = entry_t(name, info.st_mode, info.st_size);
      }
      StartCriticalSection();
      result = readdir(dir);
   }
   EndCriticalSection();
   closedir(dir);
   for (map<string,entry_t>::const_iterator it = content.begin(); it != content.end(); ++it)
      DirectoryEntry(oss, it->second, folder);
   oss << "</table></body></html>";
   response.content = oss.str();
}

void Server::DirectoryEntry(ostringstream& oss, const entry_t& entry, const string& folder)
{
   oss << "<tr><td class=\"icon\" id=\"";
   if (entry.mode & S_IFREG)
      oss << "file";
   else if (entry.mode & S_IFDIR)
      oss << "folder";
   else
      oss << "unknown";
   oss << "\"></td><td class=\"name\">";
   if ((entry.mode & S_IFDIR) || ((entry.mode & S_IFREG) && entry.size > 0))
      oss << "<a href=\"" << folder << entry.name << "\">";
   oss << entry.name;
   if ((entry.mode & S_IFDIR) || ((entry.mode & S_IFREG) && entry.size > 0))
      oss << "</a>";
   oss << "</td><td class=\"size\">";
   if (entry.mode & S_IFREG)
      oss << entry.size;
   else
      oss << "-";
   oss << "</td></tr>";
}

string Server::CreateAnchors(const string& path)
{
   if (path.length() < 5)
      return path;
   string dirs = path.substr(5);
   if (dirs.at(dirs.length()-1) == '/' || dirs.at(dirs.length()-1) == '\\')
      dirs = dirs.substr(0, dirs.length()-1);
   string rc = "<a class=\"title\" href=\"/\">ROOT</a>:";
   string::size_type pos, old_pos = 0;
   while ((pos = dirs.find('/', old_pos)) != string::npos)
   {
      rc += "<a class=\"title\" href=\"" + dirs.substr(0, pos) + "\">" + dirs.substr(old_pos, pos-old_pos) + "</a>/";
      old_pos = pos + 1;
   }
   rc += dirs.substr(old_pos);
   return rc;
}

void Server::AboutPage(args_t& /*args*/, header_t& /*header*/, response_t& response)
{
   response.content = "<html><head><title>HTTP Server</title>"
                      "<style>body { font-family: Century Gothic, sans-serif; }</style>"
                      "</head><body><h2>* Light HTTP";
#ifdef HAVE_SSL
   response.content += "/HTTPS";
#endif
   response.content += " Server *</h2></body></html>";
}

void Server::UserPage(args_t& /*args*/, header_t& header, response_t& response)
{
   string user = header["x-httpsrv-cert-cn"];
   string email = header["x-httpsrv-cert-email"];
   if (user.length() == 0)
      user = email;
   string expiry = header["x-httpsrv-cert-expiry"];
   string sni = header["x-httpsrv-sni"];
   if (user.length() > 0)
   {
      if (email.length() > 0)
         user += " - email: " + email;
      if (expiry.length() > 0)
         user += " - expiry: " + expiry;
   }
   else
   {
      user = header["x-httpsrv-username"];
      if (user.length() == 0)
         user = "not authenticated";
   }
   if (sni.length() > 0)
      user += " (SNI is " + sni + ")";
   response.content = "<html><head><title>User Info</title><style>body { font-family: Century Gothic, sans-serif; }</style></head>"
                      "<body><h2>* you are " + user + " *</h2></body></html>";
}

void Server::FormPage(args_t& /*args*/, header_t& /*header*/, response_t& response)
{
   response.content =
   "<html>\n"
   "<head><title>File Upload</title><style>body { font-family: Century Gothic, sans-serif; }</style></head>\n"
   "<body>\n"
   "  <h2>Upload File</h2>\n"
   "  <form method=\"post\" enctype=\"multipart/form-data\" action=\"upload\">\n"
   "    Choose the file to upload: <input type=\"file\" name=\"fileID\" /><br />\n"
   "    <input type=\"submit\" value=\"Upload\" />\n"
   "  </form>\n"
   "</body>\n"
   "</html>\n";
}

void Server::UploadPage(args_t& /*args*/, header_t& header, response_t& response)
{
   string fname = header["x-httpsrv-filename"];
   string tname = header["x-httpsrv-tmpfile"];
   if (fname.length() == 0 || tname.length() == 0)
   {
      response.status_code = HTTP_BAD_REQUEST;
      if (tname.length() > 0)
         unlink(tname.c_str());
      return;
   }
#ifdef MINGW
   mkdir((GetRootDir() + "uploads").c_str());
   string path = GetRootDir().substr(0, GetRootDir().length()-1) + "\\uploads\\" + fname;
#else
   mkdir((GetRootDir() + "uploads").c_str(), 0755);
   string path = GetRootDir() + "uploads/" + fname;
#endif
   response.content = "<html><head><title>Upload</title><style>body { font-family: Century Gothic, sans-serif; }</style></head>";
   if (rename(tname.c_str(), path.c_str()) != 0)
   {
      response.content += "<body><h2>Error creating file " + path + "<br>" + strerror(errno) + "</h2></body></html>";
      response.status_code = HTTP_SERVER_ERROR;
      unlink(tname.c_str());
      return;
   }
   response.content += "<body><h2>Uploaded to " + ("ROOT:uploads/" + fname) + "</h2></body></html>";
}

string Server::Now()
{
   time_t now = time(0);
   struct tm datetime;
#ifdef MINGW
   localtime_s(&datetime, &now);
#else
   localtime_r(&now, &datetime);
#endif
   ostringstream oss;
   oss << setfill('0');
   oss << (datetime.tm_year + 1900) << "-" << setw(2) << (datetime.tm_mon + 1) << "-" << setw(2) << datetime.tm_mday;
   oss << " " << setw(2) << datetime.tm_hour << ":" << setw(2) << datetime.tm_min << ":" << setw(2) << datetime.tm_sec;
   return oss.str();
}

void Server::Log(LogType type, const char *fmt,...)
{
   pthread_mutex_lock(&_log_mutex);
   char log_txt[4096];
   va_list params;
   va_start(params, fmt);
   vsnprintf(log_txt, sizeof(log_txt), fmt, params);
   va_end(params);
   ostringstream outs;

   switch (type)
   {
      case LOG_INFO:
         outs << Now() + " [INFO ] " + log_txt << endl;
         break;

      case LOG_ERROR:
         outs << Now() + " [ERROR] " + log_txt << endl;
         break;

      case LOG_DEBUG:
         outs << Now() + " [DEBUG] " + log_txt << endl;
         break;

       default:
         break;
   }
   wxThreadEvent event;
   event.SetString(outs.str().c_str());
   wxQueueEvent(_logctrl->GetParent(), event.Clone());
   //_logctrl->GetEventHandler()->CallAfter(&wxTextCtrl::AppendText, outs.str().c_str());
   pthread_mutex_unlock(&_log_mutex);
}

void Server::SetUserPassword(const std::string& user, const std::string& password)
{
   _auth_user = user;
   _auth_pwd = password;
}
