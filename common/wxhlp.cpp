#include "wxhlp.h"

using namespace std;

myFrame::myFrame(const wxString& title) : wxFrame(0, wxID_ANY, title, wxPoint(0, 0), wxSize(0, 0))
{
}

void myFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
   _main_dlg->Destroy();
   Close(true);
}

myDialog::myDialog() : wxDialog(), _info_box(0), _disable_all(0), _is_dark(false)
{
}

wxSize myDialog::ButtonSize()
{
   wxSize btnSize(-1, -1);
#if defined(MINGW) && wxMINOR_VERSION < 1
   HDC hdcMeasure = ::CreateCompatibleDC(0);
   double scaleFactor = ::GetDeviceCaps(hdcMeasure, LOGPIXELSX) / 96.0f;
   ::DeleteDC(hdcMeasure);
   if (scaleFactor != 1)
      btnSize.SetHeight(24.0 * scaleFactor);
#endif
   return btnSize;
}

void myDialog::EnableDarkMode(bool force)
{
   if (force)
      _is_dark = true;
   else
   {
#if !defined(MINGW) && wxMINOR_VERSION >= 1 && wxRELEASE_NUMBER >= 3
      _is_dark = wxSystemSettings::GetAppearance().IsDark();
#elif defined(MINGW)
      HKEY key;
      const wchar_t *reg_path = L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";
      if (RegOpenKeyExW(HKEY_CURRENT_USER, reg_path, 0, KEY_READ, &key) == NO_ERROR)
      {
         DWORD int_value;
         DWORD size = sizeof(int_value);
         if (RegQueryValueExW(key, L"AppsUseLightTheme", 0, 0, (LPBYTE)&int_value, &size) == NO_ERROR)
            _is_dark = (int_value == 0);
         RegCloseKey(key);
      }
#endif
   }
}

void myDialog::SetDarkColors(wxWindow *wnd, bool main_dlg)
{
   if (_is_dark)
   {
      if (main_dlg)
      {
         wnd->SetOwnBackgroundColour(wxColor(0, 0, 0));
         wnd->SetOwnForegroundColour(wxColor(200, 200, 200));
      }
      else
      {
         wnd->SetBackgroundColour(wxColor(0, 0, 0));
         wnd->SetForegroundColour(wxColor(200, 200, 200));
      }
   }
}

wxTextCtrl *myDialog::CreateFileInput(wxBoxSizer *row, wxSize size, int flag, int pad, const wxString& label, wxWindowID wid)
{
#if defined(MINGW) && wxMINOR_VERSION >= 1
   wxTextCtrl *txtCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, GetParent()->FromDIP(size));
#else
   wxTextCtrl *txtCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, size);
#endif
   SetDarkColors(txtCtrl);
   if (!label.empty())
   {
      wxBoxSizer *new_row = new wxBoxSizer(wxHORIZONTAL);
      wxBoxSizer *col1 = new wxBoxSizer(wxVERTICAL);
      wxBoxSizer *col2 = new wxBoxSizer(wxVERTICAL);
      wxStaticText *text = new wxStaticText(this, wxID_ANY, label);
      wxButton *btn = new wxButton(this, wid, "Browse...", wxDefaultPosition, ButtonSize());
      SetDarkColors(text);
      SetDarkColors(btn);
      col1->Add(text, 0, wxLEFT | wxBOTTOM, 2);
      col1->Add(txtCtrl, 1, wxEXPAND);
      col2->Add(0, text->GetSize().y - 1, 0, wxLEFT | wxBOTTOM, 2);
      col2->Add(btn, 0, wxLEFT, 8);
      new_row->Add(col1, 1);
      new_row->Add(col2, 0);
      row->Add(new_row, 1, flag, pad);
   }
   else
   {
      row->Add(txtCtrl, 1, flag & ~wxRIGHT, pad);
      wxButton *btn = new wxButton(this, wid, "Browse...", wxDefaultPosition, ButtonSize());
      SetDarkColors(btn);
      row->Add(btn, 0, flag, pad);
   }
   return txtCtrl;
}

wxTextCtrl *myDialog::CreateTextInput(wxBoxSizer *row, wxSize size, int flag, int pad, const wxString& label,
                                      bool expand, bool password, const wxString& defVal, bool numeric, int min, int max)
{
   long style = (password) ? wxTE_PASSWORD : wxTE_LEFT;
   wxIntegerValidator<int> valnum;
   valnum.SetRange(min, max);
   const wxValidator& val = (numeric) ? valnum : wxDefaultValidator;
#if defined(MINGW) && wxMINOR_VERSION >= 1
   wxTextCtrl *txtCtrl = new wxTextCtrl(this, wxID_ANY, defVal, wxDefaultPosition, GetParent()->FromDIP(size), style, val);
#else
   wxTextCtrl *txtCtrl = new wxTextCtrl(this, wxID_ANY, defVal, wxDefaultPosition, size, style, val);
#endif
   SetDarkColors(txtCtrl);
   if (!label.empty())
   {
      wxBoxSizer *col = new wxBoxSizer(wxVERTICAL);
      wxStaticText *text = new wxStaticText(this, wxID_ANY, label);
      SetDarkColors(text);
      col->Add(text, 0, wxLEFT | wxBOTTOM, 2);
      col->Add(txtCtrl, (expand) ? 1 : 0, wxEXPAND);
      row->Add(col, (expand) ? 1 : 0, flag, pad);
   }
   else
   {
      row->Add(txtCtrl, (expand) ? 1 : 0, flag, pad);
   }
   return txtCtrl;
}

wxCheckBox *myDialog::CreateCheckBox(wxBoxSizer *row, int flag, int pad, const wxString& label, wxWindowID id,
                                     bool shift, const bool enable)
{
   wxCheckBox *checkBox = new wxCheckBox(this, id, label, wxDefaultPosition, wxDefaultSize);
   SetDarkColors(checkBox);
   checkBox->SetValue(enable);
   if (shift)
   {
      wxBoxSizer *col = new wxBoxSizer(wxVERTICAL);
      wxStaticText *text = new wxStaticText(this, wxID_ANY, "");
      SetDarkColors(text);
      col->Add(text, 0, wxBOTTOM, 4);
      col->Add(checkBox, 0, wxALIGN_CENTER_VERTICAL);
      row->Add(col, 0, flag, pad);
   }
   else
   {
      row->Add(checkBox, 0, flag | wxALIGN_CENTER_VERTICAL, pad);
   }
   return checkBox;
}

wxRadioButton *myDialog::CreateRadioButton(wxBoxSizer *row, int flag, int pad, const wxString& label, wxWindowID id,
                                           bool shift, const bool group, const bool selected)
{
   long style = (group) ? wxRB_GROUP : 0;
   wxRadioButton *radioBtn = new wxRadioButton(this, id, label, wxDefaultPosition, wxDefaultSize, style);
   SetDarkColors(radioBtn);
   radioBtn->SetValue(selected);
   if (shift)
   {
      wxBoxSizer *col = new wxBoxSizer(wxVERTICAL);
      wxStaticText *text = new wxStaticText(this, wxID_ANY, "");
      SetDarkColors(text);
      col->Add(text);
      col->Add(radioBtn, 0, wxALIGN_CENTER_VERTICAL);
      row->Add(col, 0, flag, pad);
   }
   else
   {
      row->Add(radioBtn, 0, flag | wxALIGN_CENTER_VERTICAL, pad);
   }
   return radioBtn;
}

wxSpinCtrl *myDialog::CreateNumericInput(wxBoxSizer *row, wxSize size, int flag, int pad,
                                         const wxString& label, int defVal, int min, int max)
{
#if defined(MINGW) && wxMINOR_VERSION >= 1
   wxSpinCtrl *numCtrl = new wxSpinCtrl(this, wxID_ANY, wxString::Format("%d", defVal), wxDefaultPosition,
                                        GetParent()->FromDIP(size), wxSP_ARROW_KEYS | wxALIGN_RIGHT, min, max, defVal);
#else
   wxSpinCtrl *numCtrl = new wxSpinCtrl(this, wxID_ANY, wxString::Format("%d", defVal), wxDefaultPosition, size,
                                        wxSP_ARROW_KEYS | wxALIGN_RIGHT, min, max, defVal);
#endif
   SetDarkColors(numCtrl);
   if (!label.empty())
   {
      wxBoxSizer *col = new wxBoxSizer(wxVERTICAL);
      wxStaticText *text = new wxStaticText(this, wxID_ANY, label);
      SetDarkColors(text);
      col->Add(text, 0, wxLEFT | wxBOTTOM, 2);
      col->Add(numCtrl, 0, wxEXPAND);
      row->Add(col, 0, flag, pad);
   }
   else
   {
      row->Add(numCtrl, 0, flag, pad);
   }
   return numCtrl;
}

wxChoice *myDialog::CreateChoice(wxBoxSizer *row, int flag, int pad, const wxString& label, wxArrayString items, int selected)
{
   wxChoice *choice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, items);
   SetDarkColors(choice);
   choice->SetSelection(selected);
   if (!label.empty())
   {
      wxBoxSizer *col = new wxBoxSizer(wxVERTICAL);
      wxStaticText *text = new wxStaticText(this, wxID_ANY, label);
      SetDarkColors(text);
      col->Add(text, 0, wxLEFT | wxBOTTOM, 2);
      col->Add(choice, 0, wxEXPAND);
      row->Add(col, 0, flag, pad);
   }
   else
   {
      row->Add(choice, 0, flag, pad);
   }
   return choice;
}

wxButton *myDialog::CreateButton(wxBoxSizer *row, int flag, int pad, const wxString& label, wxWindowID id)
{
   wxButton *btn = new wxButton(this, id, label, wxDefaultPosition, ButtonSize());
   SetDarkColors(btn);
   row->Add(btn, 0, flag, pad);
   return btn;
}

wxStaticText *myDialog::CreateText(wxBoxSizer *row, int flag, int pad, const wxString& label, wxWindowID id)
{
   wxStaticText *txt = new wxStaticText(this, id, label);
   SetDarkColors(txt);
   row->Add(txt, 0, flag, pad);
   return txt;
}

void myDialog::ErrorMessage(const wxString& msg)
{
   wxMessageDialog* dialog = new wxMessageDialog(this, msg, "Error", wxCENTER | wxOK | wxICON_ERROR);
   dialog->ShowWindowModal();
}

void myDialog::WarningMessage(const wxString& msg)
{
   wxMessageDialog* dialog = new wxMessageDialog(this, msg, "Warning", wxCENTER | wxOK | wxICON_WARNING);
   dialog->ShowWindowModal();
}

void myDialog::InfoMessage(const wxString& msg)
{
   wxMessageDialog* dialog = new wxMessageDialog(this, msg, "Information", wxCENTER | wxOK | wxICON_INFORMATION);
   dialog->ShowWindowModal();
}

void myDialog::BusyStart()
{
   if (_info_box == 0)
   {
      _disable_all = new wxWindowDisabler;
      _info_box = new wxBusyInfo("Working, please wait...", this);
      wxSafeYield(this);
   }
}

void myDialog::BusyEnd()
{
   if (_info_box)
   {
      delete _info_box;
      delete _disable_all;
      _info_box = 0;
      _disable_all = 0;
   }
}

void myDialog::OnClose(wxCloseEvent& event)
{
   exit(0);
}
