#include <stdio.h>

#include "wx/wx.h"
#include "EditExpenseDialog.h"
#include "App.h"

wxBEGIN_EVENT_TABLE(EditExpenseDialog, wxDialog)
wxEND_EVENT_TABLE()

EditExpenseDialog::EditExpenseDialog(wxWindow *parent)
                 : wxDialog(parent, wxID_ANY, wxString("EditExpenseDialog")) {
    wxFlexGridSizer *gs;
    wxStaticText *stDesc, *stAmt, *stCat, *stDate;
    wxTextCtrl *tcDesc, *tcAmt, *tcCat, *tcDate;

    gs = new wxFlexGridSizer(4, 2, 0, 0);
    stDesc = new wxStaticText(this, wxID_ANY, "Description");
    stAmt = new wxStaticText(this, wxID_ANY, "Amount");
    stCat = new wxStaticText(this, wxID_ANY, "Category");
    stDate = new wxStaticText(this, wxID_ANY, "Date");
    tcDesc = new wxTextCtrl(this, wxID_ANY, "");
    tcAmt = new wxTextCtrl(this, wxID_ANY, "");
    tcCat = new wxTextCtrl(this, wxID_ANY, "");
    tcDate = new wxTextCtrl(this, wxID_ANY, "");

    gs->Add(stDesc, 0, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(tcDesc, 1, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(stAmt, 0, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(tcAmt, 1, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(stCat, 0, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(tcCat, 1, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(stDate, 0, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(tcDate, 1, wxALIGN_CENTER_VERTICAL, 0);

    SetSizer(gs);
    gs->Fit(this);
    gs->SetSizeHints(this);
}

