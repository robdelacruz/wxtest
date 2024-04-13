#include <stdio.h>

#include "wx/wx.h"
#include "wx/datectrl.h"
#include "EditExpenseDialog.h"
#include "App.h"

wxBEGIN_EVENT_TABLE(EditExpenseDialog, wxDialog)
wxEND_EVENT_TABLE()

EditExpenseDialog::EditExpenseDialog(wxWindow *parent)
                 : wxDialog(parent, wxID_ANY, wxString("New Expense")) {
    wxBoxSizer *top;
    wxBoxSizer *vbox;
    wxBoxSizer *hbox;
    wxStaticBoxSizer *staticbox;
    wxFlexGridSizer *gs;
    wxStaticText *stDesc, *stAmt, *stCat, *stDate;
    wxTextCtrl *tcDesc, *tcAmt, *tcCat;
    wxDatePickerCtrl *dpDate;
    wxButton *btnOK, *btnCancel;
    wxStaticBox *sb;

    staticbox = new wxStaticBoxSizer(wxVERTICAL, this, "Expense Details");
    //staticbox = new wxStaticBoxSizer(wxVERTICAL, this, "");
    sb = staticbox->GetStaticBox();
    stDesc = new wxStaticText(sb, wxID_ANY, "Description");
    stAmt = new wxStaticText(sb, wxID_ANY, "Amount");
    stCat = new wxStaticText(sb, wxID_ANY, "Category");
    stDate = new wxStaticText(sb, wxID_ANY, "Date");
    tcDesc = new wxTextCtrl(sb, wxID_ANY, "", wxDefaultPosition, wxSize(200,-1));
    tcDesc->SetMaxLength(30);
    tcAmt = new wxTextCtrl(sb, wxID_ANY, "");
    tcCat = new wxTextCtrl(sb, wxID_ANY, "");
    dpDate = new wxDatePickerCtrl(sb, wxID_ANY);

    gs = new wxFlexGridSizer(4, 2, 5, 5);
    gs->Add(stDesc, 0, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(tcDesc, 1, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(stAmt, 0, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(tcAmt, 1, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(stCat, 0, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(tcCat, 1, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(stDate, 0, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(dpDate, 1, wxALIGN_CENTER_VERTICAL, 0);
    //staticbox->Add(gs, 1, wxEXPAND|wxALL, 10);
    staticbox->AddSpacer(5);
    staticbox->Add(gs, 1, wxEXPAND|wxLEFT|wxRIGHT, 10);
    staticbox->AddSpacer(10);

    hbox = new wxBoxSizer(wxHORIZONTAL);
    btnOK = new wxButton(this, wxID_OK, "OK");
    btnCancel = new wxButton(this, wxID_CANCEL, "Cancel");
    hbox->Add(btnOK, 0, 0, 0);
    hbox->AddSpacer(10);
    hbox->Add(btnCancel, 0, 0, 0);

    vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(staticbox, 0, wxEXPAND, 0);
    vbox->AddSpacer(15);
    vbox->Add(hbox, 0, wxALIGN_CENTER, 0);

    top = new wxBoxSizer(wxVERTICAL);
    top->Add(vbox, 1, wxEXPAND|wxALL, 10);

    SetSizer(top);
    top->Fit(this);
}

