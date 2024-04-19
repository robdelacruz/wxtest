#include <stdio.h>
#include <assert.h>

#include "wx/wx.h"
#include "wx/valgen.h"
#include "wx/valnum.h"
#include "wx/datectrl.h"

#include "EditExpenseDialog.h"
#include "App.h"

wxBEGIN_EVENT_TABLE(EditExpenseDialog, wxDialog)
wxEND_EVENT_TABLE()

EditExpenseDialog::EditExpenseDialog(wxWindow *parent, exp_t *xp)
                 : wxDialog(parent, wxID_ANY, wxString("New Expense")) {
    ExpenseContext *ctx = getContext();

    m_desc = wxString::FromUTF8(xp->desc->s);
    m_amt = xp->amt;
    m_date = wxDateTime(xp->date);
    m_xp = xp;

    m_icatsel = -1;
    for (size_t i=0; i < ctx->cats->len; i++) {
        cat_t *cat = (cat_t *) ctx->cats->items[i];
        if (cat->catid == xp->catid) {
            m_icatsel = i;
            break;
        }
    }

    CreateControls();
}

void EditExpenseDialog::CreateControls() {
    ExpenseContext *ctx = getContext();
    wxBoxSizer *top;
    wxBoxSizer *vbox;
    wxStdDialogButtonSizer *btnbox;
    wxStaticBoxSizer *staticbox;
    wxFlexGridSizer *gs;
    wxStaticText *stDesc, *stAmt, *stCat, *stDate;
    wxTextCtrl *tcDesc, *tcAmt;
    wxChoice *choiceCat;
    wxButton *btnOK, *btnCancel;
    wxStaticBox *sb;
    wxTextValidator vldDesc(wxFILTER_NONE, &m_desc);
    wxFloatingPointValidator<double> vldAmt(2, &m_amt, wxNUM_VAL_ZERO_AS_BLANK | wxNUM_VAL_THOUSANDS_SEPARATOR);
    wxGenericValidator vldCat(&m_icatsel);
    wxArrayString choices;
    wxDatePickerCtrl *dpDate;

    DestroyChildren();
    SetSizer(NULL);

    for (size_t i=0; i < ctx->cats->len; i++) {
        cat_t *cat = (cat_t *) ctx->cats->items[i];
        choices.Add(wxString::FromUTF8(cat->name->s));
    }

    staticbox = new wxStaticBoxSizer(wxVERTICAL, this, "Expense Details");
    sb = staticbox->GetStaticBox();

    stDesc = new wxStaticText(sb, wxID_ANY, "Description");
    stAmt = new wxStaticText(sb, wxID_ANY, "Amount");
    stCat = new wxStaticText(sb, wxID_ANY, "Category");
    stDate = new wxStaticText(sb, wxID_ANY, "Date");

    tcDesc = new wxTextCtrl(sb, ID_EDITEXPENSE_DESCRIPTION, "", wxDefaultPosition, wxSize(200,-1), 0, vldDesc);
    tcDesc->SetMaxLength(30);
    tcAmt = new wxTextCtrl(sb, ID_EDITEXPENSE_AMOUNT, "", wxDefaultPosition, wxDefaultSize, 0, vldAmt);
    choiceCat = new wxChoice(sb, ID_EDITEXPENSE_CATEGORY, wxDefaultPosition, wxDefaultSize, choices, 0, vldCat);
    dpDate = new wxDatePickerCtrl(sb, ID_EDITEXPENSE_DATE, m_date, wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT|wxDP_SHOWCENTURY);

    btnOK = new wxButton(this, wxID_OK, "OK");
    btnCancel = new wxButton(this, wxID_CANCEL, "Cancel");

    gs = new wxFlexGridSizer(4, 2, 5, 5);
    gs->Add(stDesc, 0, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(tcDesc, 1, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(stAmt, 0, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(tcAmt, 1, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(stCat, 0, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(choiceCat, 1, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(stDate, 0, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(dpDate, 1, wxALIGN_CENTER_VERTICAL, 0);
    staticbox->AddSpacer(5);
    staticbox->Add(gs, 1, wxEXPAND|wxLEFT|wxRIGHT, 10);
    staticbox->AddSpacer(10);

    btnbox = new wxStdDialogButtonSizer();
    btnbox->AddButton(btnOK);
    btnbox->AddButton(btnCancel);
    btnbox->Realize();

    vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(staticbox, 0, wxEXPAND, 0);
    vbox->AddSpacer(15);
    vbox->Add(btnbox, 0, wxEXPAND, 0);

    top = new wxBoxSizer(wxVERTICAL);
    top->Add(vbox, 1, wxEXPAND|wxALL, 10);

    SetSizer(top);
    top->Fit(this);
}

bool EditExpenseDialog::TransferDataFromWindow() {
    wxDatePickerCtrl *datectrl = (wxDatePickerCtrl *) wxWindow::FindWindowById(ID_EDITEXPENSE_DATE);
    uint64_t catid;

    wxDialog::TransferDataFromWindow();

    if (m_icatsel == -1)
        return false;
    if (m_desc.Length() == 0)
        return false;

    catid = GetCatIdFromSelectedIndex(m_icatsel);
    if (catid == 0)
        return false;

    str_assign(m_xp->desc, m_desc.mb_str(wxConvUTF8));
    m_xp->amt = m_amt;
    m_xp->catid = catid;
    m_xp->date = datectrl->GetValue().GetTicks();

    return true;
}

uint64_t EditExpenseDialog::GetCatIdFromSelectedIndex(int icatsel) {
    ExpenseContext *ctx = getContext();
    cat_t *cat;

    if (icatsel < 0)
        return 0;
    if (icatsel > (int) ctx->cats->len-1)
        return 0;

    cat = (cat_t *) ctx->cats->items[icatsel];
    return cat->catid;
}


