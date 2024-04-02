#include <stdio.h>

#include "wx/wx.h"
#include "wx/window.h"
#include "wx/filename.h"
#include "App.h"
#include "Frame.h"
#include "db.h"
#include "art/home.xpm"
#include "art/back.xpm"
#include "art/forward.xpm"

#define ID_PREV_MONTH 101
#define ID_NEXT_MONTH 102
#define ID_EXPENSES_LIST 103
#define ID_EXPENSES_HEADING 104
#define ID_EXPENSE_NEW 105
#define ID_EXPENSE_EDIT 106
#define ID_EXPENSE_DEL 107

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_NEW, MyFrame::OnFileNew)
    EVT_MENU(wxID_OPEN, MyFrame::OnFileOpen)
    EVT_MENU(wxID_CLOSE, MyFrame::OnFileClose)
    EVT_MENU(wxID_EXIT, MyFrame::OnFileExit)
    EVT_BUTTON(ID_PREV_MONTH, MyFrame::OnPrevMonth)
    EVT_BUTTON(ID_NEXT_MONTH, MyFrame::OnNextMonth)
    EVT_LIST_ITEM_ACTIVATED(ID_EXPENSES_LIST, MyFrame::OnListItemActivated)
END_EVENT_TABLE()

MyFrame::MyFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(640,480)) {
    SetIcon(wxIcon(home_xpm));

    CreateMenuBar();
    CreateStatusBar(2);
    SetStatusText(wxT("Welcome to wxWidgets."));
    CreateControls();

    RefreshControls();
    RefreshExpenses();
}

void MyFrame::CreateMenuBar() {
    wxMenuBar *mb;
    wxMenu *menu;
    ExpenseContext *ctx = getContext();
    int expfile_open = ctx_is_open_expfile(ctx);

    mb = GetMenuBar();
    if (mb) {
        SetMenuBar(NULL);
        mb->Destroy();
    }

    mb = new wxMenuBar();
    menu = new wxMenu;
    menu->Append(wxID_NEW, wxT("&New\tCtrl-Shift-N"), wxT("New Expense File"));
    menu->Append(wxID_OPEN, wxT("&Open\tCtrl-O"), wxT("Open Expense File"));
    if (expfile_open)
        menu->Append(wxID_CLOSE, wxT("&Close\tCtrl-W"), wxT("Close Expense File"));
    menu->Append(wxID_EXIT, wxT("E&xit\tCtrl-Q"), wxT("Quit program"));
    mb->Append(menu, wxT("&File"));

    if (expfile_open) {
        menu = new wxMenu;
        menu->Append(ID_EXPENSE_NEW, wxT("&New\tCtrl-N"), wxT("New Expense"));
        menu->Append(ID_EXPENSE_EDIT, wxT("&Edit\tCtrl-E"), wxT("Edit Expense"));
        menu->Append(ID_EXPENSE_DEL, wxT("&Delete\tCtrl-X"), wxT("Delete Expense"));
        mb->Append(menu, wxT("&Expense"));
    }
    SetMenuBar(mb);
}

void MyFrame::CreateControls() {
    wxListView *lv;
    wxBitmapButton *btnPrev, *btnNext;
    wxBoxSizer *vbox;
    wxBoxSizer *hbox;

    DestroyChildren();
    SetSizer(NULL);

    //lv = new wxListView(this, wxID_ANY, wxDefaultPosition, wxSize(250,200));
    lv = new wxListView(this, ID_EXPENSES_LIST);
    lv->AppendColumn("Date");
    lv->AppendColumn("Description");
    lv->AppendColumn("Amount");
    lv->AppendColumn("Category");

    for (int col=0; col <= 3; col++)
        lv->SetColumnWidth(col, wxLIST_AUTOSIZE_USEHEADER);

    lv->SetColumnWidth(1, 200);

    btnPrev = new wxBitmapButton(this, ID_PREV_MONTH, wxBitmap(back_xpm));
    btnNext = new wxBitmapButton(this, ID_NEXT_MONTH, wxBitmap(forward_xpm));
    hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(btnPrev, 0, wxEXPAND, 0);
    hbox->AddStretchSpacer();
    hbox->Add(new wxStaticText(this, ID_EXPENSES_HEADING, ""), 0, wxALIGN_CENTER_VERTICAL, 0);
    hbox->AddStretchSpacer();
    hbox->Add(btnNext, 0, wxEXPAND, 0);

    vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(hbox, 0, wxEXPAND|wxTOP|wxBOTTOM, 5);
    vbox->Add(lv, 1, wxEXPAND, 0);
    SetSizer(vbox);
//    vbox->Fit(this);
//    vbox->SetSizeHints(this);
}

void MyFrame::RefreshControls() {
    ExpenseContext *ctx = getContext();
    wxWindow *lv, *btnPrev, *btnNext;
    wxStaticText *st;
    wxString title, filename, ext;

    CreateControls();

    st = (wxStaticText *) wxWindow::FindWindowById(ID_EXPENSES_HEADING);
    lv = wxWindow::FindWindowById(ID_EXPENSES_LIST);
    btnPrev = wxWindow::FindWindowById(ID_PREV_MONTH);
    btnNext = wxWindow::FindWindowById(ID_NEXT_MONTH);

    // No open expense file
    if (!ctx_is_open_expfile(ctx)) {
        SetTitle(wxT("Expense Buddy"));
        st->SetLabel(wxT("No Expense File"));
        lv->Show(false);
        btnPrev->Show(false);
        btnNext->Show(false);
        return;
    }

    // Expense file opened
    wxFileName::SplitPath(wxString::FromUTF8(ctx->expfile->s), NULL, NULL, &filename, &ext);
    title.Printf("Expense Buddy - [%s.%s]", filename, ext);
    SetTitle(title);
    lv->Show(true);
    btnPrev->Show(true);
    btnNext->Show(true);
}

void MyFrame::RefreshExpenses() {
    ExpenseContext *ctx = getContext();
    wxStaticText *st;
    wxListView *lv;
    exp_t *xp;
    char buf[24];

    if (!ctx_is_open_expfile(ctx))
        return;

    st = (wxStaticText *) wxWindow::FindWindowById(ID_EXPENSES_HEADING, this);
    date_strftime(ctx->dt, "%B %Y", buf, sizeof(buf));
    st->SetLabel(wxString::FromUTF8(buf));

    ctx_refresh_expenses(ctx);

    lv = (wxListView *) wxWindow::FindWindowById(ID_EXPENSES_LIST, this);
    lv->DeleteAllItems();

    for (size_t i=0; i < ctx->xps->len; i++) {
        xp = (exp_t *) ctx->xps->items[i];
        date_strftime(xp->date, "%m-%d", buf, sizeof(buf));
        lv->InsertItem(i, buf);
        lv->SetItem(i, 1, xp->desc->s);
        snprintf(buf, sizeof(buf), "%9.2f", xp->amt);
        lv->SetItem(i, 2, buf);
        lv->SetItem(i, 3, xp->catname->s);
    }
}

static wxString fileErrorString(wxString& file, int errnum) {
    wxString errstr;
    errstr.Printf("%s: %s", wxString::FromUTF8(exp_strerror(errnum)), file);
    return errstr;
}

void MyFrame::OnFileNew(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    int z;

    wxFileDialog dlg(this, wxT("New Expense File"), wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_SAVE, wxDefaultPosition, wxDefaultSize);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;

    wxString expfile = dlg.GetPath();
    z = ctx_create_expense_file(ctx, expfile.ToUTF8());
    if (z != 0) {
        wxMessageDialog msgdlg(NULL, fileErrorString(expfile, z), wxT("Error occured"), wxOK | wxICON_ERROR);
        msgdlg.ShowModal();
        return;
    }

    CreateMenuBar();
    RefreshControls();
    RefreshExpenses();
}
void MyFrame::OnFileOpen(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    int z;

    wxFileDialog dlg(this, wxT("Open Expense File"), wxEmptyString, wxEmptyString, wxT("*.db"), wxFD_OPEN, wxDefaultPosition, wxDefaultSize);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;

    wxString expfile = dlg.GetPath();
    z = ctx_open_expense_file(ctx, expfile.ToUTF8());
    if (z != 0) {
        wxMessageDialog msgdlg(NULL, fileErrorString(expfile, z), wxT("Error occured"), wxOK | wxICON_ERROR);
        msgdlg.ShowModal();
        return;
    }

    CreateMenuBar();
    RefreshControls();
    RefreshExpenses();
}
void MyFrame::OnFileClose(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    ctx_close(ctx);

    CreateMenuBar();
    RefreshControls();
    RefreshExpenses();
}
void MyFrame::OnFileExit(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    ctx_close(ctx);
    Close();
}

void MyFrame::OnPrevMonth(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    ctx_set_prev_month(ctx);
    RefreshExpenses();
}
void MyFrame::OnNextMonth(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    ctx_set_next_month(ctx);
    RefreshExpenses();
}
void MyFrame::OnListItemActivated(wxListEvent& event) {
    long i = event.GetIndex();
    printf("list activated GetIndex(): %ld\n", i);
    wxListItem item = event.GetItem();
    wxString s = event.GetText();
    printf("on list selected: '%s'\n", s.mb_str().data());
}

