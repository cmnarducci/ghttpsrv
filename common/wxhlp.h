#ifndef __WXHLP_H__
#define __WXHLP_H__
#if defined(MACOS) && defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpotentially-evaluated-expression"
#endif
#include <wx/wx.h>
#if defined(MACOS) && defined(__clang__)
#pragma clang diagnostic pop
#endif
#include <wx/spinctrl.h>
#include <wx/busyinfo.h>
#include <wx/valnum.h>

class myFrame: public wxFrame
{
public:
   myFrame(const wxString& title);
   void SetMainDialog(wxDialog *dlg) { _main_dlg = dlg; };
   wxDialog *GetMainDialog() { return _main_dlg; };

protected:
   virtual void OnExit(wxCommandEvent& event);
   wxDialog *_main_dlg;
};

class myDialog: public wxDialog
{
public:
   myDialog();
   bool IsDarkMode();

protected:
   virtual void OnClose(wxCloseEvent& event);
   virtual void ErrorMessage(const wxString& msg);
   virtual void WarningMessage(const wxString& msg);
   virtual void InfoMessage(const wxString& msg);
   void BusyStart();
   void BusyEnd();
   wxSize ButtonSize();
   wxTextCtrl *CreateTextInput(wxBoxSizer *row, wxSize size, int flag, int pad, const wxString& label = wxT(""),
                               bool expand = false, bool password = false, const wxString& defVal = wxT(""),
                               bool numeric = false, int min = 1, int max = 65535);
   wxTextCtrl *CreateFileInput(wxBoxSizer *row, wxSize size, int flag, int pad, const wxString& label, wxWindowID wid = wxID_ANY);
   wxSpinCtrl *CreateNumericInput(wxBoxSizer *row, wxSize size, int flag, int pad,
                                  const wxString& label, int defVal = 0, int min = 0, int max = 9999999);
   wxCheckBox *CreateCheckBox(wxBoxSizer *row, int flag, int pad, const wxString& label, wxWindowID id = wxID_ANY,
                              bool shift = false, const bool enable = false);
   wxRadioButton *CreateRadioButton(wxBoxSizer *row, int flag, int pad, const wxString& label, wxWindowID id = wxID_ANY,
                                    bool shift = false, const bool group = false, const bool selected = false);
   wxChoice *CreateChoice(wxBoxSizer *row, int flag, int pad, const wxString& label, wxArrayString items, int selected = 0);
   wxButton *CreateButton(wxBoxSizer *row, int flag, int pad, const wxString& label, wxWindowID id = wxID_ANY);
   wxStaticText *CreateText(wxBoxSizer *row, int flag, int pad, const wxString& label = wxT(""), wxWindowID id = wxID_ANY);

private:
   wxBusyInfo *_info_box;
   wxWindowDisabler *_disable_all;
};
#endif
