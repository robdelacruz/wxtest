#include "wx/wx.h"
#include <assert.h>

#include "SetupCategoriesDialog.h"
#include "App.h"

wxBEGIN_EVENT_TABLE(SetupCategoriesDialog, wxDialog)
wxEND_EVENT_TABLE()

SetupCategoriesDialog::SetupCategoriesDialog(wxWindow *parent)
                 : wxDialog(parent, wxID_ANY, wxString("Setup Categories")) {

    CreateControls();
}

void SetupCategoriesDialog::CreateControls() {
    ExpenseContext *ctx = getContext();
    wxBoxSizer *top;
    wxBoxSizer *vbox;
    wxBoxSizer *hbox;
    wxBoxSizer *vboxLeft;
    wxBoxSizer *vboxRight;
    wxButton *btnAdd, *btnEdit, *btnDelete, *btnClose;
    wxStaticText *st;
    wxListBox *lb;
    wxArrayString cats;

    DestroyChildren();
    SetSizer(NULL);

    st = new wxStaticText(this, wxID_ANY, wxString::Format("Number of categories: %ld", ctx->cats->len));

    for (size_t i=0; i < ctx->cats->len; i++) {
        cat_t *cat = (cat_t *) ctx->cats->items[i];
        cats.Add(wxString::FromUTF8(cat->name->s));
    }
    lb = new wxListBox(this, ID_SETUPCATEGORIES_LIST, wxDefaultPosition, wxSize(200,200), cats, wxLB_SINGLE);

    btnAdd = new wxButton(this, wxID_ADD, "&Add", wxDefaultPosition, wxSize(-1, BTN_HEIGHT));
    btnEdit = new wxButton(this, wxID_EDIT, "&Edit", wxDefaultPosition, wxSize(-1, BTN_HEIGHT));
    btnDelete = new wxButton(this, wxID_DELETE, "&Delete", wxDefaultPosition, wxSize(-1, BTN_HEIGHT));
    btnClose = new wxButton(this, wxID_CANCEL, "&Close", wxDefaultPosition, wxSize(-1, BTN_HEIGHT));

    vboxLeft = new wxBoxSizer(wxVERTICAL);
    vboxLeft->Add(lb, 0, wxEXPAND, 0);

    vboxRight = new wxBoxSizer(wxVERTICAL);
    vboxRight->Add(btnAdd, 0, wxEXPAND, 0);
    vboxRight->AddSpacer(5);
    vboxRight->Add(btnEdit, 0, wxEXPAND, 0);
    vboxRight->AddSpacer(5);
    vboxRight->Add(btnDelete, 0, wxEXPAND, 0);
    vboxRight->AddSpacer(5);
    vboxRight->Add(btnClose, 0, wxEXPAND, 0);

    hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(vboxLeft, 0, wxEXPAND, 0);
    hbox->AddSpacer(10);
    hbox->Add(vboxRight, 0, wxFIXED_MINSIZE, 0);

    vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(st, 0, wxEXPAND, 0);
    vbox->AddSpacer(10);
    vbox->Add(hbox, 1, wxEXPAND, 0);

    top = new wxBoxSizer(wxVERTICAL);
    top->Add(vbox, 0, wxEXPAND | wxALL, 10);

    SetSizer(top);
    top->Fit(this);
}

