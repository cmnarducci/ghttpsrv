#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <iostream>
#include <fstream>
#include <sstream>
#include "main.h"
#ifndef MINGW
#include "app.xpm"
#endif

using namespace std;

wxIMPLEMENT_APP(HttpSrvApp);

bool HttpSrvApp::OnInit()
{
   MainFrame *frame = new MainFrame("HTTP/HTTPS Server");
   MainDialog *dlg = new MainDialog();
   frame->Show(false);
   frame->SetMainDialog(dlg);
   SetTopWindow(frame);
   dlg->Show(true);
   dlg->SetInitialFocus();
   return true;
}

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
   EVT_MENU(wxID_EXIT,  MainFrame::OnExit)
   EVT_MENU(ID_Exit,  MainFrame::OnExit)
   EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(const wxString& title) : myFrame(title)
{
   wxMenu *menuFile = new wxMenu;
   menuFile->Append(ID_Exit, "E&xit...", "");
   menuFile->Append(wxID_ABOUT);
   wxMenuBar *menuBar = new wxMenuBar;
   menuBar->Append(menuFile, "&File");
   SetMenuBar(menuBar);
}

void MainFrame::OnExit(wxCommandEvent& event)
{
   Tools::Sleepms(250);
   myFrame::OnExit(event);
}

void MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
   wxString version = "Version 3.0.0";
#ifdef HAVE_SSL
   wxMessageBox(version, "HTTP/HTTPS Server", wxOK | wxICON_INFORMATION);
#else
   wxMessageBox(version, "HTTP Server", wxOK | wxICON_INFORMATION);
#endif
}

wxIMPLEMENT_CLASS(MainDialog, wxDialog);

wxBEGIN_EVENT_TABLE(MainDialog, wxDialog)
   EVT_CLOSE(MainDialog::OnClose)
   EVT_CHECKBOX(ID_SSL, MainDialog::OnClick)
   EVT_CHECKBOX(ID_Auth, MainDialog::OnClick)
   EVT_RADIOBUTTON(ID_BasicAuth, MainDialog::OnClick)
   EVT_RADIOBUTTON(ID_CertAuth, MainDialog::OnClick)
   EVT_BUTTON(ID_BrowseRootFolder, MainDialog::OnBrowseRootFolder)
   EVT_BUTTON(ID_BrowseCaCertificate, MainDialog::OnBrowseCaFile)
   EVT_BUTTON(ID_BrowseCertificate, MainDialog::OnBrowseCertFile)
   EVT_BUTTON(ID_Start, MainDialog::OnStart)
   EVT_BUTTON(ID_Stop, MainDialog::OnStop)
   EVT_THREAD(wxID_ANY, MainDialog::OnLogUpdate)
wxEND_EVENT_TABLE()

void MainDialog::OnClose(wxCloseEvent& event)
{
   if (_httpsrv.IsRunning())
   {
      _httpsrv.End();
      _httpsrv.Wait();
      _httpsrv.Log(Server::LOG_INFO, "Server stopped");
      Tools::Sleepms(200);
   }
   BaseSock::Cleanup();
   exit(0);
}

MainDialog::MainDialog() : myDialog()
{
#if defined(MINGW) && wxMINOR_VERSION >= 1
   cpad = FromDIP(10);
#else
   cpad = 10;
#endif
#ifdef __WXMAC__
   const int fs = 10;
#else
   const int fs = 8;
#endif
   BaseSock::Setup();
   SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
#ifdef __WXMAC__
   wxSizerFlags::DisableConsistencyChecks();
#endif
#ifdef HAVE_SSL
   wxString title = "HTTP/HTTPS Server v3.0.0";
#else
   wxString title = "HTTP Server v3.0.0";
#endif
   Create(0, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,
          wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMINIMIZE_BOX | wxMAXIMIZE_BOX);

   // SetTransparent(240);
   wxBoxSizer *blocks = new wxBoxSizer(wxVERTICAL);
   {
      wxBoxSizer *block = new wxStaticBoxSizer(wxVERTICAL, this, "Server");
      wxBoxSizer *row1 = new wxBoxSizer(wxHORIZONTAL);
      map<string,vector<string> > ifaces;
      BaseSock::GetLocalAddresses(ifaces);
      wxArrayString ips;
      ips.Add("Any");
      for (map<string,vector<string> >::const_iterator iface = ifaces.begin(); iface != ifaces.end(); ++iface)
      {
         for (vector<string>::const_iterator ip = (*iface).second.begin(); ip != (*iface).second.end(); ++ip)
            ips.Add((*ip));
      }
      _bind_ip = CreateChoice(row1, wxLEFT | wxTOP, cpad, "Bind to IP", ips);
#if defined(MINGW) || defined(MACOS)
      _port = CreateNumericInput(row1, wxSize(80, -1), wxLEFT | wxRIGHT | wxTOP, cpad, "Port", 80, 1, 65535);
#else
      _port = CreateTextInput(row1, wxSize(80, -1), wxLEFT | wxRIGHT | wxTOP, cpad, "Port", false, false, "80", true);
#endif
      _ssl = CreateCheckBox(row1, wxLEFT | wxTOP, cpad, " Enable SSL", ID_SSL, true);
      _cert_pass = CreateTextInput(row1, wxSize(120, -1), wxLEFT | wxTOP | wxRIGHT, cpad, "PK Passphrase", false, true, "");
      wxBoxSizer *row2 = new wxBoxSizer(wxHORIZONTAL);
      _cert_file = CreateFileInput(row2, wxSize(420, -1), wxLEFT | wxTOP | wxRIGHT, cpad,
                                   "PEM file (Certificate + Private Key) ", ID_BrowseCertificate);
      wxBoxSizer *row3 = new wxBoxSizer(wxHORIZONTAL);
      _root_dir = CreateFileInput(row3, wxSize(420, -1), wxALL, cpad, "Root Folder", ID_BrowseRootFolder);
      block->Add(row1, 0, wxEXPAND);
      block->Add(row2, 0, wxEXPAND);
      block->Add(row3, 0, wxEXPAND);
      blocks->Add(block, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, cpad);
   }
   {
      wxBoxSizer *block = new wxStaticBoxSizer(wxVERTICAL, this, "Authentication");
      wxBoxSizer *row1 = new wxBoxSizer(wxHORIZONTAL);
      _auth = CreateCheckBox(row1, wxLEFT | wxTOP | wxRIGHT, cpad, " Enable Authentication", ID_Auth);
      _basic_auth = CreateRadioButton(row1, wxLEFT | wxTOP, cpad, " Basic", ID_BasicAuth, false, true, true);
      _cert_auth = CreateRadioButton(row1, wxLEFT | wxTOP | wxRIGHT, cpad, " Using Certificate", ID_CertAuth);
      wxBoxSizer *row2 = new wxBoxSizer(wxHORIZONTAL);
      _username = CreateTextInput(row2, wxSize(140, -1), wxLEFT | wxTOP, cpad, "Username");
      _password = CreateTextInput(row2, wxSize(140, -1), wxLEFT | wxTOP | wxRIGHT, cpad, "Password", false, true);
      wxBoxSizer *row3 = new wxBoxSizer(wxHORIZONTAL);
      _ca_file = CreateFileInput(row3, wxSize(420, -1), wxALL, cpad, "CA Certificates File", ID_BrowseCaCertificate);
      block->Add(row1, 0, wxEXPAND);
      block->Add(row2, 0, wxEXPAND);
      block->Add(row3, 0, wxEXPAND);
      blocks->Add(block, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, cpad);
   }
   {
      wxBoxSizer *block = new wxStaticBoxSizer(wxVERTICAL, this, "Control");
      wxBoxSizer *row = new wxBoxSizer(wxHORIZONTAL);
      CreateButton(row, wxTOP | wxLEFT | wxBOTTOM, cpad, "Start", ID_Start);
      wxButton* stop = CreateButton(row, wxALL, cpad, "Stop", ID_Stop);
      block->Add(row);
      blocks->Add(block, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, cpad);
      stop->Enable(false);
   }
   {
      wxBoxSizer *row = new wxBoxSizer(wxHORIZONTAL);
#if defined(MACOS)
      _logs = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(610, 210), wxTE_MULTILINE);
#elif defined(MINGW) && wxMINOR_VERSION >= 1
      _logs = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, GetParent()->FromDIP(wxSize(610, 210)),
                             wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2);
#else
      _logs = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(610, 210), wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2);
#endif
      _logs->SetFont(wxFont(wxFontInfo(fs).FaceName("Courier New")));
      row->Add(_logs, 1, wxEXPAND);
      blocks->Add(row, 1, wxEXPAND | wxALL, 10);
      _httpsrv.SetLogCtrl(_logs);
   }

   SetEscapeId(wxID_NONE);
   SetIcon(wxICON(wxICON_AAA));
   CheckEnable();
   SetSizerAndFit(blocks);
}

wxString MainDialog::BrowseSelectFile()
{
   static wxString extDef;
   wxString wild = wxString::Format("All files (%s)|%s", wxFileSelectorDefaultWildcardStr, wxFileSelectorDefaultWildcardStr);
   int flags = wxFD_OPEN | wxFD_CHANGE_DIR | wxFD_PREVIEW;
   wxString path = wxFileSelector("Select the file to load", wxEmptyString, wxEmptyString, extDef, wild, flags, this);
   if (path.length() > 0)
      extDef = path.AfterLast('.');
   return path;
}

wxString MainDialog::BrowseSelectFolder()
{
   long flags = wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST;
   wxString path = wxDirSelector("Select the folder", wxEmptyString, flags, wxDefaultPosition, this);
   return path;
}

void MainDialog::CheckEnable()
{
#ifndef HAVE_SSL
   _ssl->Enable(false);
#endif
   bool ssl = _ssl->IsChecked();
   if (!ssl)
      _basic_auth->SetValue(true);
   bool auth = _auth->IsChecked();
   bool basic = _basic_auth->GetValue();
   _cert_pass->Enable(ssl);
   _cert_file->Enable(ssl);
   _basic_auth->Enable(auth);
   _cert_auth->Enable(ssl && auth);
   _username->Enable(auth && basic);
   _password->Enable(auth && basic);
   _ca_file->Enable(ssl && auth && !basic);
   FindWindowById(ID_BrowseCaCertificate, this)->Enable(ssl && auth && !basic);
   FindWindowById(ID_BrowseCertificate, this)->Enable(ssl);
}

void MainDialog::OnClick(wxCommandEvent& event)
{
   CheckEnable();
}

void MainDialog::OnBrowseRootFolder(wxCommandEvent& event)
{
   wxString path = BrowseSelectFolder();
   if (path.length() > 0)
      _root_dir->SetValue(path);
}

void MainDialog::OnBrowseCaFile(wxCommandEvent& event)
{
   wxString path = BrowseSelectFile();
   if (path.length() > 0)
      _ca_file->SetValue(path);
}

void MainDialog::OnBrowseCertFile(wxCommandEvent& event)
{
   wxString path = BrowseSelectFile();
   if (path.length() > 0)
      _cert_file->SetValue(path);
}

void MainDialog::OnStart(wxCommandEvent& event)
{
#if defined(MINGW) || defined(MACOS)
   ostringstream svc;
   svc << _port->GetValue();
   string service(svc.str());
#else
   string service(_port->GetValue().c_str());
#endif
   bool auth = _auth->IsChecked();
   bool basic = _basic_auth->GetValue();
   string proto = (_ssl->IsChecked()) ? "HTTPS" : "HTTP";
   string source_ip(_bind_ip->GetStringSelection());
   if (source_ip == "Any")
      source_ip = "0.0.0.0";
   string root_dir(_root_dir->GetValue().c_str());
   if (root_dir == ".")
      root_dir = "";
   string cert_file = (_ssl->IsChecked()) ? string(_cert_file->GetValue().c_str()) : "";
   string pk_pass = (_ssl->IsChecked()) ? string(_cert_pass->GetValue().c_str()) : "";
   string ca_file = (auth & !basic) ? string(_ca_file->GetValue().c_str()) : "";
   string username = (auth & basic) ? string(_username->GetValue().c_str()) : "";
   string password = (auth & basic) ? string(_password->GetValue().c_str()) : "";

   _httpsrv.SetServerAddress(source_ip);
   _httpsrv.SetServerPort(service);
#ifdef HAVE_SSL
   _httpsrv.SetSSL(false);
   if (cert_file.length() > 0)
      _httpsrv.UseCertificate(cert_file, cert_file, pk_pass);
   if (_ssl->IsChecked() && auth && !basic && ca_file.length() == 0)
   {
      ErrorMessage("CA file not specified");
      return;
   }
   if (_ssl->IsChecked() && !_httpsrv.SetSSL(true, true, BaseSock::TLS, ca_file))
   {
      wxString msg = "Error loading ";
      if (cert_file.length() > 0)
      {
         msg += "SSL certificate";
         if (ca_file.length() > 0)
            msg += " or CA certificate";
      }
      else if (ca_file.length() > 0)
         msg += "CA certificate";
      ErrorMessage(msg);
      return;
   }
#endif
   _httpsrv.SetRootDir(root_dir);
   _httpsrv.SetUserPassword(username, password);
   _httpsrv.Start();
   Tools::Sleepms(100);
   if (!_httpsrv.IsRunning())
   {
      _httpsrv.Log(Server::LOG_ERROR, "Server not running, error # %d (%s)", _httpsrv.GetErrorNumber(),
                                                                             _httpsrv.GetErrorMessage().c_str());
   }
   else
   {
      _httpsrv.Log(Server::LOG_INFO, "%s Server listening on %s port %s", proto.c_str(), source_ip.c_str(), service.c_str());
      FindWindowById(ID_Start, this)->Enable(false);
      FindWindowById(ID_Stop, this)->Enable(true);
   }
}

void MainDialog::OnStop(wxCommandEvent& event)
{
   _httpsrv.End();
   _httpsrv.Wait();
   _httpsrv.Log(Server::LOG_INFO, "Server stopped");
   FindWindowById(ID_Start, this)->Enable(true);
   FindWindowById(ID_Stop, this)->Enable(false);
}

void MainDialog::OnLogUpdate(wxThreadEvent& event)
{
   _logs->AppendText(event.GetString());
}
