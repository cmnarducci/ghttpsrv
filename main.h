#ifndef __MAIN_H__
#define __MAIN_H__
#include "wxhlp.h"
#ifdef MINGW
#undef _WINSOCKAPI_
#endif
#include "server.h"
#include "tools.h"

enum
{
   ID_Exit = 1,
   ID_SSL = 10001,
   ID_BrowseRootFolder,
   ID_BrowseCaCertificate,
   ID_BrowseCertificate,
   ID_Auth,
   ID_BasicAuth = 11001,
   ID_CertAuth,
   ID_Start = 12001,
   ID_Stop
};

class HttpSrvApp: public wxApp
{
public:
   virtual bool OnInit();
};

class MainFrame: public myFrame
{
public:
   MainFrame(const wxString& title);

protected:
   virtual void OnExit(wxCommandEvent& event);

private:
   void OnAbout(wxCommandEvent& event);
   wxDECLARE_EVENT_TABLE();
};

class MainDialog: public myDialog
{
   wxDECLARE_CLASS(MainDialog);
public:
   MainDialog();
   void SetInitialFocus() { _bind_ip->SetFocus(); };

private:
   wxString BrowseSelectFile();
   wxString BrowseSelectFolder();
   void OnClose(wxCloseEvent& event);
   void CheckEnable();
   void OnClick(wxCommandEvent& event);
   void OnBrowseRootFolder(wxCommandEvent& event);
   void OnBrowseCaFile(wxCommandEvent& event);
   void OnBrowseCertFile(wxCommandEvent& event);
   void OnStart(wxCommandEvent& event);
   void OnStop(wxCommandEvent& event);
   void OnLogUpdate(wxThreadEvent& event);
   int cpad;
   wxChoice *_bind_ip;
   wxTextCtrl *_root_dir;
   wxSpinCtrl *_port;
   wxCheckBox *_ssl, *_auth;
   wxRadioButton *_basic_auth, *_cert_auth;
   wxTextCtrl *_username, *_password;
   wxTextCtrl *_ca_file;
   wxTextCtrl *_cert_file;
   wxTextCtrl *_cert_pass;
   wxTextCtrl *_logs;
   Server _httpsrv;

   wxDECLARE_EVENT_TABLE();
};
#endif
