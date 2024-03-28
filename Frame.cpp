#include <stdio.h>

#include "wx/wx.h"
#include "wx/window.h"
#include "App.h"
#include "Frame.h"
#include "db.h"
#include "art/home.xpm"
#include "art/back.xpm"
#include "art/forward.xpm"

#define ID_PREV_MONTH 101
#define ID_NEXT_MONTH 102
#define ID_EXPENSES_LIST 103
#define ID_FRAME_TITLE 104

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_NEW, MyFrame::OnFileNew)
    EVT_MENU(wxID_OPEN, MyFrame::OnFileOpen)
    EVT_MENU(wxID_EXIT, MyFrame::OnFileExit)
    EVT_BUTTON(ID_PREV_MONTH, MyFrame::OnPrevMonth)
    EVT_BUTTON(ID_NEXT_MONTH, MyFrame::OnNextMonth)
    EVT_LIST_ITEM_ACTIVATED(ID_EXPENSES_LIST, MyFrame::OnListItemActivated)
END_EVENT_TABLE()

MyFrame::MyFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title) {
    wxMenuBar *menubar;
    wxMenu *fileMenu;
    wxListView *lv;
    wxBitmapButton *btnPrevMonth, *btnNextMonth;
    wxBoxSizer *vbox;
    wxBoxSizer *hbox;

    SetIcon(wxIcon(home_xpm));

    fileMenu = new wxMenu;
    fileMenu->Append(wxID_NEW, wxT("&New\tCtrl-Shift-N"), wxT("New Expense File"));
    fileMenu->Append(wxID_OPEN, wxT("&Open\tCtrl-O"), wxT("Open Expense File"));
    fileMenu->Append(wxID_EXIT, wxT("E&xit\tCtrl-Q"), wxT("Quit program"));

    menubar = new wxMenuBar();
    menubar->Append(fileMenu, wxT("&File"));
    SetMenuBar(menubar);

    CreateStatusBar(2);
    SetStatusText(wxT("Welcome to wxWidgets."));

    //lv = new wxListView(this, wxID_ANY, wxDefaultPosition, wxSize(250,200));
    lv = new wxListView(this, ID_EXPENSES_LIST);
    lv->AppendColumn("Date");
    lv->AppendColumn("Description");
    lv->AppendColumn("Amount");
    lv->AppendColumn("Category");

    for (int col=0; col <= 3; col++)
        lv->SetColumnWidth(col, wxLIST_AUTOSIZE_USEHEADER);

    lv->SetColumnWidth(1, 200);

    btnPrevMonth = new wxBitmapButton(this, ID_PREV_MONTH, wxBitmap(back_xpm));
    btnNextMonth = new wxBitmapButton(this, ID_NEXT_MONTH, wxBitmap(forward_xpm));
    hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(btnPrevMonth, 0, wxEXPAND, 0);
    hbox->AddStretchSpacer();
    hbox->Add(new wxStaticText(this, ID_FRAME_TITLE, "Expenses"), 0, wxALIGN_CENTER_VERTICAL, 0);
    hbox->AddStretchSpacer();
    hbox->Add(btnNextMonth, 0, wxEXPAND, 0);

    vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(hbox, 0, wxEXPAND|wxTOP|wxBOTTOM, 5);
    vbox->Add(lv, 1, wxEXPAND, 0);
    SetSizer(vbox);
    vbox->Fit(this);
//    vbox->SetSizeHints(this);

    RefreshExpenses();
}

void MyFrame::RefreshExpenses() {
    ExpenseContext *ctx = getContext();
    wxStaticText *st;
    wxListView *lv;
    exp_t *xp;
    char buf[24];

    if (!ctx_is_open_expfile(ctx))
        return;

    st = (wxStaticText *) wxWindow::FindWindowById(ID_FRAME_TITLE, this);
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

void MyFrame::OnFileNew(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    wxFileDialog *dlg;
    int z;

    dlg = new wxFileDialog(this, wxT("New Expense File"), wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_OPEN, wxDefaultPosition, wxDefaultSize);
    z = dlg->ShowModal();
    if (z == wxID_OK) {
        str_t *err = str_new(0);
        wxString expfile = dlg->GetPath();
        if (ctx_create_expense_file(ctx, expfile.ToUTF8(), err) != 0) {
            wxMessageDialog msgdlg(NULL, wxT("Error occured"), wxString::FromUTF8(err->s), wxOK | wxICON_ERROR);
            msgdlg.ShowModal();
        }
    }

    dlg->Destroy();
}
void MyFrame::OnFileOpen(wxCommandEvent& event) {
//    ExpenseContext *ctx = getContext();
}
void MyFrame::OnFileExit(wxCommandEvent& event) {
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

