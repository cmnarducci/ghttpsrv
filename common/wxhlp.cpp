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

myDialog::myDialog() : wxDialog(), _info_box(0), _disable_all(0)
{
}

wxSize myDialog::ButtonSize()
{
   wxSize btnSize(-1, -1);
#if defined(MINGW) && ((wxMINOR_VERSION < 1 && wxMAJOR_VERSION == 3) || wxMAJOR_VERSION < 3)
   HDC hdcMeasure = ::CreateCompatibleDC(0);
   double scaleFactor = ::GetDeviceCaps(hdcMeasure, LOGPIXELSX) / 96.0f;
   ::DeleteDC(hdcMeasure);
   if (scaleFactor != 1)
      btnSize.SetHeight(24.0 * scaleFactor);
#endif
   return btnSize;
}

wxTextCtrl *myDialog::CreateFileInput(wxBoxSizer *row, wxSize size, int flag, int pad, const wxString& label, wxWindowID wid)
{
#if defined(MINGW) && ((wxMINOR_VERSION >= 1 && wxMAJOR_VERSION == 3) || wxMAJOR_VERSION > 3)
   int cpad = GetParent()->FromDIP(pad);
   wxTextCtrl *txtCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, GetParent()->FromDIP(size));
#else
   int cpad = pad;
   wxTextCtrl *txtCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, size);
#endif
   wxBoxSizer *new_row = new wxBoxSizer(wxHORIZONTAL);
   wxBoxSizer *col1 = new wxBoxSizer(wxVERTICAL);
   wxBoxSizer *col2 = new wxBoxSizer(wxVERTICAL);
   wxStaticText *text = 0;
   if (!label.empty())
   {
      text = new wxStaticText(this, wxID_ANY, label);
      col1->Add(text, 0, wxLEFT | wxBOTTOM, 2);
   }
   wxButton *btn = new wxButton(this, wid, "Browse...", wxDefaultPosition, ButtonSize());
   col1->Add(txtCtrl, 1, wxEXPAND);
#if defined(MINGW)
   const int adjust = -4;
#elif defined(MACOS)
   const int adjust = 1;
#else
   const int adjust = 0;
#endif
   if (!label.empty())
      col2->Add(0, text->GetSize().y + adjust, 0, wxLEFT | wxBOTTOM, 2);
   else
      col2->Add(0, adjust, 0, wxLEFT | wxBOTTOM, 2);
   col2->Add(btn, 0, wxLEFT, 8);
   new_row->Add(col1, 1);
   new_row->Add(col2, 0);
   row->Add(new_row, 1, flag, cpad);
   return txtCtrl;
}

wxTextCtrl *myDialog::CreateTextInput(wxBoxSizer *row, wxSize size, int flag, int pad, const wxString& label,
                                      bool expand, bool password, const wxString& defVal, bool numeric, int min, int max)
{
   long style = (password) ? wxTE_PASSWORD : wxTE_LEFT;
   wxIntegerValidator<int> valnum;
   valnum.SetRange(min, max);
   const wxValidator& val = (numeric) ? valnum : wxDefaultValidator;
#if defined(MINGW) && ((wxMINOR_VERSION >= 1 && wxMAJOR_VERSION == 3) || wxMAJOR_VERSION > 3)
   int cpad = GetParent()->FromDIP(pad);
   wxTextCtrl *txtCtrl = new wxTextCtrl(this, wxID_ANY, defVal, wxDefaultPosition, GetParent()->FromDIP(size), style, val);
#else
   int cpad = pad;
   wxTextCtrl *txtCtrl = new wxTextCtrl(this, wxID_ANY, defVal, wxDefaultPosition, size, style, val);
#endif
   if (!label.empty())
   {
      wxBoxSizer *col = new wxBoxSizer(wxVERTICAL);
      wxStaticText *text = new wxStaticText(this, wxID_ANY, label);
      col->Add(text, 0, wxLEFT | wxBOTTOM, 2);
      col->Add(txtCtrl, (expand) ? 1 : 0, wxEXPAND);
      row->Add(col, (expand) ? 1 : 0, flag, cpad);
   }
   else
   {
      row->Add(txtCtrl, (expand) ? 1 : 0, flag, cpad);
   }
   return txtCtrl;
}

wxCheckBox *myDialog::CreateCheckBox(wxBoxSizer *row, int flag, int pad, const wxString& label, wxWindowID id,
                                     bool shift, const bool enable)
{
#if defined(MINGW) && ((wxMINOR_VERSION >= 1 && wxMAJOR_VERSION == 3) || wxMAJOR_VERSION > 3)
   int cpad = GetParent()->FromDIP(pad);
#else
   int cpad = pad;
#endif
   wxCheckBox *checkBox = new wxCheckBox(this, id, label, wxDefaultPosition, wxDefaultSize);
   checkBox->SetValue(enable);
   if (shift)
   {
      wxBoxSizer *col = new wxBoxSizer(wxVERTICAL);
      wxStaticText *text = new wxStaticText(this, wxID_ANY, "");
      col->Add(text, 0, wxBOTTOM, 4);
      col->Add(checkBox, 0, wxALIGN_CENTER_VERTICAL);
      row->Add(col, 0, flag, cpad);
   }
   else
   {
      row->Add(checkBox, 0, flag | wxALIGN_CENTER_VERTICAL, cpad);
   }
   return checkBox;
}

wxRadioButton *myDialog::CreateRadioButton(wxBoxSizer *row, int flag, int pad, const wxString& label, wxWindowID id,
                                           bool shift, const bool group, const bool selected)
{
#if defined(MINGW) && ((wxMINOR_VERSION >= 1 && wxMAJOR_VERSION == 3) || wxMAJOR_VERSION > 3)
   int cpad = GetParent()->FromDIP(pad);
#else
   int cpad = pad;
#endif
   long style = (group) ? wxRB_GROUP : 0;
   wxRadioButton *radioBtn = new wxRadioButton(this, id, label, wxDefaultPosition, wxDefaultSize, style);
   radioBtn->SetValue(selected);
   if (shift)
   {
      wxBoxSizer *col = new wxBoxSizer(wxVERTICAL);
      wxStaticText *text = new wxStaticText(this, wxID_ANY, "");
      col->Add(text);
      col->Add(radioBtn, 0, wxALIGN_CENTER_VERTICAL);
      row->Add(col, 0, flag, cpad);
   }
   else
   {
      row->Add(radioBtn, 0, flag | wxALIGN_CENTER_VERTICAL, cpad);
   }
   return radioBtn;
}

wxSpinCtrl *myDialog::CreateNumericInput(wxBoxSizer *row, wxSize size, int flag, int pad,
                                         const wxString& label, int defVal, int min, int max)
{
   wxPanel *panel = new wxPanel(this, wxID_ANY);
#if defined(MINGW) && ((wxMINOR_VERSION >= 1 && wxMAJOR_VERSION == 3) || wxMAJOR_VERSION > 3)
   int cpad = GetParent()->FromDIP(pad);
   wxSpinCtrl *numCtrl = new wxSpinCtrl(panel, wxID_ANY, wxString::Format("%d", defVal), wxDefaultPosition,
                                        GetParent()->FromDIP(size), wxSP_ARROW_KEYS | wxALIGN_RIGHT, min, max, defVal);
#else
   int cpad = pad;
   wxSpinCtrl *numCtrl = new wxSpinCtrl(panel, wxID_ANY, wxString::Format("%d", defVal), wxDefaultPosition, size,
                                        wxSP_ARROW_KEYS | wxALIGN_RIGHT, min, max, defVal);
#endif
   wxBoxSizer *col = new wxBoxSizer(wxVERTICAL);
   if (!label.empty())
   {
      wxStaticText *text = new wxStaticText(panel, wxID_ANY, label);
      col->Add(text, 0, wxLEFT | wxBOTTOM, 2);
   }
   col->Add(numCtrl, 0, wxEXPAND);
   panel->SetSizer(col);
   row->Add(panel, 0, flag, cpad);
   return numCtrl;
}

wxChoice *myDialog::CreateChoice(wxBoxSizer *row, int flag, int pad, const wxString& label, wxArrayString items, int selected)
{
#if defined(MINGW) && ((wxMINOR_VERSION >= 1 && wxMAJOR_VERSION == 3) || wxMAJOR_VERSION > 3)
   int cpad = GetParent()->FromDIP(pad);
#else
   int cpad = pad;
#endif
   wxChoice *choice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, items);
   choice->SetSelection(selected);
   if (!label.empty())
   {
      wxBoxSizer *col = new wxBoxSizer(wxVERTICAL);
      wxStaticText *text = new wxStaticText(this, wxID_ANY, label);
      col->Add(text, 0, wxLEFT | wxBOTTOM, 2);
      col->Add(choice, 0, wxEXPAND);
      row->Add(col, 0, flag, cpad);
   }
   else
   {
      row->Add(choice, 0, flag, cpad);
   }
   return choice;
}

wxButton *myDialog::CreateButton(wxBoxSizer *row, int flag, int pad, const wxString& label, wxWindowID id)
{
#if defined(MINGW) && ((wxMINOR_VERSION >= 1 && wxMAJOR_VERSION == 3) || wxMAJOR_VERSION > 3)
   int cpad = GetParent()->FromDIP(pad);
#else
   int cpad = pad;
#endif
   wxButton *btn = new wxButton(this, id, label, wxDefaultPosition, ButtonSize());
   row->Add(btn, 0, flag, cpad);
   return btn;
}

wxStaticText *myDialog::CreateText(wxBoxSizer *row, int flag, int pad, const wxString& label, wxWindowID id)
{
#if defined(MINGW) && ((wxMINOR_VERSION >= 1 && wxMAJOR_VERSION == 3) || wxMAJOR_VERSION > 3)
   int cpad = GetParent()->FromDIP(pad);
#else
   int cpad = pad;
#endif
   wxStaticText *txt = new wxStaticText(this, id, label);
   row->Add(txt, 0, flag, cpad);
   return txt;
}

bool myDialog::IsDarkMode()
{
#if (wxMINOR_VERSION >= 1 && wxMAJOR_VERSION == 3) || wxMAJOR_VERSION > 3
   return wxSystemSettings::GetAppearance().IsDark();
#else
   return false;
#endif
}

void myDialog::ErrorMessage(const wxString& msg)
{
   wxMessageDialog* dialog = new wxMessageDialog(this, msg, "Error", wxCENTER | wxOK | wxICON_ERROR);
   dialog->ShowModal();
}

void myDialog::WarningMessage(const wxString& msg)
{
   wxMessageDialog* dialog = new wxMessageDialog(this, msg, "Warning", wxCENTER | wxOK | wxICON_WARNING);
   dialog->ShowModal();
}

void myDialog::InfoMessage(const wxString& msg)
{
   wxMessageDialog* dialog = new wxMessageDialog(this, msg, "Information", wxCENTER | wxOK | wxICON_INFORMATION);
   dialog->ShowModal();
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

void myDialog::OnClose(wxCloseEvent&)
{
   exit(0);
}
