#include "wx/wx.h"
#include <assert.h>

#include "SetupCategoriesDialog.h"
#include "App.h"

wxBEGIN_EVENT_TABLE(SetupCategoriesDialog, wxDialog)
    EVT_LISTBOX(ID_SETUPCATEGORIES_LIST, SetupCategoriesDialog::OnListBoxSelected)
    EVT_BUTTON(wxID_ADD, SetupCategoriesDialog::OnAdd)
    EVT_BUTTON(wxID_EDIT, SetupCategoriesDialog::OnEdit)
    EVT_BUTTON(wxID_DELETE, SetupCategoriesDialog::OnDelete)
wxEND_EVENT_TABLE()

SetupCategoriesDialog::SetupCategoriesDialog(wxWindow *parent)
                 : wxDialog(parent, wxID_ANY, wxString("Setup Categories")) {

    CreateControls();
    Refresh(0, 0);
    ShowControls();
}

void SetupCategoriesDialog::CreateControls() {
    wxBoxSizer *top;
    wxBoxSizer *vbox;
    wxBoxSizer *hbox;
    wxBoxSizer *vboxLeft;
    wxBoxSizer *vboxRight;
    wxButton *btnAdd, *btnEdit, *btnDelete, *btnClose;
    wxStaticText *st;
    wxListBox *lb;

    DestroyChildren();
    SetSizer(NULL);

    st = new wxStaticText(this, ID_SETUPCATEGORIES_HEADING, "");
    lb = new wxListBox(this, ID_SETUPCATEGORIES_LIST, wxDefaultPosition, wxSize(200,200), 0, NULL, wxLB_SINGLE);
    btnAdd = new wxButton(this, wxID_ADD, "&Add", wxDefaultPosition, wxSize(-1, BTN_HEIGHT));
    btnEdit = new wxButton(this, wxID_EDIT, "&Edit", wxDefaultPosition, wxSize(-1, BTN_HEIGHT));
    btnDelete = new wxButton(this, wxID_DELETE, "&Delete", wxDefaultPosition, wxSize(-1, BTN_HEIGHT));
    btnClose = new wxButton(this, wxID_CANCEL, "&Close", wxDefaultPosition, wxSize(-1, BTN_HEIGHT));
    btnClose->SetDefault();
    btnClose->SetFocus();

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

void SetupCategoriesDialog::ShowControls() {
    wxListBox *lb = (wxListBox *) wxWindow::FindWindowById(ID_SETUPCATEGORIES_LIST, this);
    wxButton *btnEdit = (wxButton *) wxWindow::FindWindowById(wxID_EDIT, this);
    wxButton *btnDelete = (wxButton *) wxWindow::FindWindowById(wxID_DELETE, this);

    if (lb->GetSelection() == wxNOT_FOUND) {
        btnEdit->Enable(false);
        btnDelete->Enable(false);
    } else {
        btnEdit->Enable(true);
        btnDelete->Enable(true);
    }
}

// To select row number: Refresh(0, rownum)
// To select expense id: Refresh(catid, 0)
void SetupCategoriesDialog::Refresh(uint64_t sel_catid, int sel_row) {
    ExpenseContext *ctx = getContext();
    wxListBox *lb = (wxListBox *) wxWindow::FindWindowById(ID_SETUPCATEGORIES_LIST, this);
    wxStaticText *st = (wxStaticText *) wxWindow::FindWindowById(ID_SETUPCATEGORIES_HEADING, this);
    wxArrayString cats;

    for (size_t i=0; i < ctx->cats->len; i++) {
        cat_t *cat = (cat_t *) ctx->cats->items[i];
        cats.Add(wxString::FromUTF8(cat->name->s));

        if (sel_catid != 0 && sel_catid == cat->catid)
            sel_row = i;
    }

    st->SetLabel(wxString::Format("Number of categories: %ld", ctx->cats->len));
    lb->Clear();
    lb->InsertItems(cats, 0);

    // Restore previous row selection.
    if (sel_row >= 0 && (size_t)sel_row < ctx->cats->len) {
        lb->SetSelection(sel_row);
        lb->EnsureVisible(sel_row);
    }
}

void SetupCategoriesDialog::OnListBoxSelected(wxCommandEvent& event) {
    ShowControls();
}

void SetupCategoriesDialog::OnAdd(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    cat_t *cat;
    uint64_t existingid;

    wxTextEntryDialog dlg(this, wxString::Format("New category"));
    if (dlg.ShowModal() != wxID_OK)
        return;

    wxString newCatName = dlg.GetValue();
    if (newCatName.Length() == 0)
        return;
    db_find_cat_by_name(ctx->expfiledb, newCatName.utf8_str(), &existingid);
    if (existingid != 0) {
        wxMessageDialog dlg(this, wxString::Format("'%s' already exists", newCatName));
        dlg.ShowModal();
        return;
    }

    cat = cat_new();
    str_assign(cat->name, newCatName.utf8_str());
    db_add_cat(ctx->expfiledb, cat);

    ctx_refresh_categories(ctx);
    Refresh(cat->catid, 0);
    ShowControls();

    cat_free(cat);
}
void SetupCategoriesDialog::OnEdit(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    wxListBox *lb = (wxListBox *) wxWindow::FindWindowById(ID_SETUPCATEGORIES_LIST, this);
    int isel = lb->GetSelection();
    cat_t *cat;
    uint64_t existingid;

    if (isel == wxNOT_FOUND)
        return;
    if (isel < 0 || (size_t)isel > ctx->cats->len-1)
        return;
    cat = (cat_t *) ctx->cats->items[isel];

    wxTextEntryDialog dlg(this, "Rename category", "Category Setup", wxString::FromUTF8(cat->name->s));
    if (dlg.ShowModal() != wxID_OK)
        return;

    wxString newCatName = dlg.GetValue();
    if (newCatName.Length() == 0)
        return;
    if (newCatName == wxString::FromUTF8(cat->name->s))
        return;
    db_find_cat_by_name(ctx->expfiledb, newCatName.utf8_str(), &existingid);
    if (existingid != 0) {
        wxMessageDialog dlg(this, wxString::Format("'%s' already exists", newCatName));
        dlg.ShowModal();
        return;
    }

    str_assign(cat->name, newCatName.utf8_str());
    db_edit_cat(ctx->expfiledb, cat);

    ctx_refresh_categories(ctx);
    Refresh(cat->catid, 0);
    ShowControls();
}
void SetupCategoriesDialog::OnDelete(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    wxListBox *lb = (wxListBox *) wxWindow::FindWindowById(ID_SETUPCATEGORIES_LIST, this);
    int isel = lb->GetSelection();
    cat_t *cat;
    long num_cat_xps = 0;
    wxString confirmMsg;

    if (isel == wxNOT_FOUND)
        return;
    if (isel < 0 || (size_t)isel > ctx->cats->len-1)
        return;
    cat = (cat_t *) ctx->cats->items[isel];

    db_count_exp_with_catid(ctx->expfiledb, cat->catid, &num_cat_xps);

    if (num_cat_xps > 0) {
        confirmMsg.Printf("%ld items will be reset to Uncategorized.\n\nDelete category '%s'?", num_cat_xps, wxString::FromUTF8(cat->name->s));
    } else {
        confirmMsg.Printf("Delete category '%s'?", wxString::FromUTF8(cat->name->s));
    }

    wxMessageDialog dlg(this, confirmMsg, "Confirm Delete", wxYES_NO | wxNO_DEFAULT); 
    if (dlg.ShowModal() != wxID_YES)
        return;
    ctx_delete_category(ctx, cat->catid);

    ctx_refresh_categories(ctx);
    if ((size_t)isel > ctx->cats->len-1)
        isel = ctx->cats->len-1;
    Refresh(0, isel);
    ShowControls();
}
