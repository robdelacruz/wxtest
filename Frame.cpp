#include <stdio.h>

#include "wx/wx.h"
#include "wx/window.h"
#include "wx/listctrl.h"
#include "wx/splitter.h"
#include "wx/filename.h"
#include "App.h"
#include "Frame.h"
#include "db.h"
#include "art/home.xpm"
#include "art/back.xpm"
#include "art/forward.xpm"

enum {
    ID_EXPENSE_NEW = 101,
    ID_EXPENSE_EDIT,
    ID_EXPENSE_DEL,

    ID_EXPENSES_PREV,
    ID_EXPENSES_NEXT,
    ID_EXPENSES_FILTERTEXT,

    ID_EXPENSES_LIST,
    ID_EXPENSES_HEADING,
    ID_EXPENSES_PANEL,

    ID_COUNT
};

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_NEW, MyFrame::OnFileNew)
    EVT_MENU(wxID_OPEN, MyFrame::OnFileOpen)
    EVT_MENU(wxID_CLOSE, MyFrame::OnFileClose)
    EVT_MENU(wxID_EXIT, MyFrame::OnFileExit)
    EVT_BUTTON(ID_EXPENSES_PREV, MyFrame::OnPrevMonth)
    EVT_BUTTON(ID_EXPENSES_NEXT, MyFrame::OnNextMonth)
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
    wxSplitterWindow *hsplitter;
    wxPanel *pnlExpensesView;

    DestroyChildren();
    SetSizer(NULL);

    hsplitter = new wxSplitterWindow(this, wxID_ANY, wxPoint(0,0), wxSize(400,400), wxSP_3D);
    pnlExpensesView = CreateExpensesView(hsplitter);

    hsplitter->Initialize(pnlExpensesView);

//    vbox->Fit(this);
//    vbox->SetSizeHints(this);
}

wxPanel* MyFrame::CreateExpensesNav(wxWindow *parent) {
    return NULL;
}

wxPanel* MyFrame::CreateExpensesView(wxWindow *parent) {
    wxPanel *pnl;
    wxListView *lv;
    wxBitmapButton *btnPrev, *btnNext;
    wxTextCtrl *txtFilter;
    wxBoxSizer *vbox;
    wxBoxSizer *hbox;
    wxListItem colAmount;

    pnl = new wxPanel(parent, ID_EXPENSES_PANEL);

    //lv = new wxListView(pnl, wxID_ANY, wxDefaultPosition, wxSize(250,200));
    lv = new wxListView(pnl, ID_EXPENSES_LIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    lv->AppendColumn("Date");
    lv->AppendColumn("Description");
    lv->AppendColumn("Amount");
    lv->AppendColumn("Category");

    lv->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
    lv->SetColumnWidth(1, 200);
    lv->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
    lv->SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);

    lv->GetColumn(2, colAmount);
    colAmount.SetAlign(wxLIST_FORMAT_RIGHT);
    lv->SetColumn(2, colAmount);

    btnPrev = new wxBitmapButton(pnl, ID_EXPENSES_PREV, wxBitmap(back_xpm));
    btnNext = new wxBitmapButton(pnl, ID_EXPENSES_NEXT, wxBitmap(forward_xpm));
    hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(btnPrev, 0, wxALIGN_CENTER, 0);
    hbox->AddSpacer(5);
    hbox->Add(new wxStaticText(pnl, ID_EXPENSES_HEADING, ""), 0, wxALIGN_CENTER, 0);
    hbox->AddSpacer(5);
    hbox->Add(btnNext, 0, wxALIGN_CENTER, 0);
    hbox->AddStretchSpacer();
    hbox->Add(new wxStaticText(pnl, wxID_ANY, "Filter:"), 0, wxALIGN_CENTER, 0);
    hbox->AddSpacer(5);
    txtFilter = new wxTextCtrl(pnl, ID_EXPENSES_FILTERTEXT, wxT(""), wxDefaultPosition, wxSize(300,-1));
    hbox->Add(txtFilter, 0, wxALIGN_CENTER, 0);

    vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(hbox, 0, wxEXPAND|wxTOP|wxBOTTOM, 5);
    vbox->Add(lv, 1, wxEXPAND, 0);
    pnl->SetSizer(vbox);

    return pnl;
}

void MyFrame::RefreshControls() {
    ExpenseContext *ctx = getContext();
    wxWindow *lv, *btnPrev, *btnNext;
    wxStaticText *st;
    wxString title, filename, ext;

    CreateControls();

    st = (wxStaticText *) wxWindow::FindWindowById(ID_EXPENSES_HEADING);
    lv = wxWindow::FindWindowById(ID_EXPENSES_LIST);
    btnPrev = wxWindow::FindWindowById(ID_EXPENSES_PREV);
    btnNext = wxWindow::FindWindowById(ID_EXPENSES_NEXT);

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
    wxPanel *expenses_panel;
    exp_t *xp;
    char buf[24];

    if (!ctx_is_open_expfile(ctx))
        return;

    st = (wxStaticText *) wxWindow::FindWindowById(ID_EXPENSES_HEADING, this);
    date_strftime(ctx->dt, "%b %Y", buf, sizeof(buf));
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

    expenses_panel = (wxPanel *) wxWindow::FindWindowById(ID_EXPENSES_PANEL);
    expenses_panel->Layout();
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

