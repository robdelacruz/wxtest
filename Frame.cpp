#include <stdio.h>

#include "wx/wx.h"
#include "wx/window.h"
#include "wx/listctrl.h"
#include "wx/splitter.h"
#include "wx/filename.h"
#include "wx/propgrid/propgrid.h"
#include "wx/propgrid/advprops.h"

#include "art/home.xpm"
#include "art/back.xpm"
#include "art/forward.xpm"

#include "App.h"
#include "Frame.h"
#include "EditExpenseDialog.h"
#include "db.h"

enum {
    ID_EXPENSE_NEW = wxID_HIGHEST,
    ID_EXPENSE_EDIT,
    ID_EXPENSE_DEL,

    ID_MAIN_SPLIT,
    ID_EXP_SPLIT,

    ID_EXPENSES_PANEL,
    ID_EXPENSES_HEADING,
    ID_EXPENSES_LIST,
    ID_EXPENSE_GRID,

    ID_EXPENSES_PREV,
    ID_EXPENSES_NEXT,
    ID_EXPENSES_FILTERTEXT,

    ID_COUNT
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_NEW, MyFrame::OnFileNew)
    EVT_MENU(wxID_OPEN, MyFrame::OnFileOpen)
    EVT_MENU(wxID_CLOSE, MyFrame::OnFileClose)
    EVT_MENU(wxID_EXIT, MyFrame::OnFileExit)
    EVT_BUTTON(ID_EXPENSES_PREV, MyFrame::OnPrevMonth)
    EVT_BUTTON(ID_EXPENSES_NEXT, MyFrame::OnNextMonth)
    EVT_LIST_ITEM_SELECTED(ID_EXPENSES_LIST, MyFrame::OnListItemSelected)
    EVT_LIST_ITEM_ACTIVATED(ID_EXPENSES_LIST, MyFrame::OnListItemActivated)
    EVT_PG_CHANGED(ID_EXPENSE_GRID, MyFrame::OnPropertyGridChanged)
wxEND_EVENT_TABLE()

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
    wxSplitterWindow *splMain;
    wxSplitterWindow *splExp;
    wxPanel *pnlExpensesView;
    wxPropertyGrid *pgExpense;

    DestroyChildren();
    SetSizer(NULL);

    splMain = new wxSplitterWindow(this, ID_MAIN_SPLIT, wxPoint(0,0), wxDefaultSize, wxSP_3D);

    splExp = new wxSplitterWindow(splMain, ID_EXP_SPLIT, wxPoint(0,0), wxSize(400,400), wxSP_3D);
    pnlExpensesView = CreateExpensesView(splExp);
    pgExpense = CreateExpensePropGrid(splExp);
    splExp->SplitHorizontally(pnlExpensesView, pgExpense);
    splExp->SetSashPosition(200);

    splMain->Initialize(splExp);

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
    lv = new wxListView(pnl, ID_EXPENSES_LIST, wxDefaultPosition, wxSize(400,200), wxLC_REPORT);
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

wxPropertyGrid* MyFrame::CreateExpensePropGrid(wxWindow *parent) {
    wxPropertyGrid *pg;
    wxArrayString cats;
    wxArrayInt idcats;
    ExpenseContext *ctx = getContext();
    cat_t *cat;

    for (size_t i=0; i < ctx->cats->len; i++) {
        cat = (cat_t *)ctx->cats->items[i];
        idcats.Add((int)cat->catid);
        cats.Add(wxString(cat->name->s));
    }

    pg = new wxPropertyGrid(parent, ID_EXPENSE_GRID, wxDefaultPosition, wxDefaultSize, wxPG_SPLITTER_AUTO_CENTER | wxPG_DEFAULT_STYLE);
    pg->Append(new wxPropertyCategory("Expense"));
    pg->Append(new wxStringProperty("Description", wxPG_LABEL));
    pg->Append(new wxFloatProperty("Amount", wxPG_LABEL));
    pg->Append(new wxEnumProperty("Category", wxPG_LABEL, cats, idcats));
    pg->Append(new wxDateProperty("Date", wxPG_LABEL));

    return pg;
}

void MyFrame::RefreshControls() {
    ExpenseContext *ctx = getContext();
    wxSplitterWindow *mainsplit;
//    wxSplitterWindow *expsplit;
    wxString title, filename, ext;

    mainsplit = (wxSplitterWindow *) wxWindow::FindWindowById(ID_MAIN_SPLIT);
//    expsplit = (wxSplitterWindow *) wxWindow::FindWindowById(ID_EXP_SPLIT);

    // No open expense file
    if (!ctx_is_open_expfile(ctx)) {
        SetTitle(wxT("Expense Buddy"));
        mainsplit->Show(false);
        return;
    }

    // Expense file opened
    wxFileName::SplitPath(wxString::FromUTF8(ctx->expfile->s), NULL, NULL, &filename, &ext);
    title.Printf("Expense Buddy - [%s.%s]", filename, ext);
    SetTitle(title);
    mainsplit->Show(true);
}

void MyFrame::RefreshExpenses() {
    ExpenseContext *ctx = getContext();
    wxStaticText *st;
    wxListView *lv;
    wxPanel *expenses_panel;
    exp_t *xp;
    char buf[24];

    lv = (wxListView *) wxWindow::FindWindowById(ID_EXPENSES_LIST, this);
    lv->DeleteAllItems();

    if (!ctx_is_open_expfile(ctx))
        return;

    st = (wxStaticText *) wxWindow::FindWindowById(ID_EXPENSES_HEADING, this);
    date_strftime(ctx->dt, "%b %Y", buf, sizeof(buf));
    st->SetLabel(wxString::FromUTF8(buf));

    ctx_refresh_expenses(ctx);

    for (size_t i=0; i < ctx->xps->len; i++) {
        xp = (exp_t *) ctx->xps->items[i];

        date_strftime(xp->date, "%m-%d", buf, sizeof(buf));
        lv->InsertItem(i, buf);
        lv->SetItem(i, 1, xp->desc->s);
        snprintf(buf, sizeof(buf), "%9.2f", xp->amt);
        lv->SetItem(i, 2, buf);
        lv->SetItem(i, 3, xp->catname->s);

        lv->SetItemPtrData(i, (wxUIntPtr)xp);
    }
    // Select first row
    if (ctx->xps->len > 0)
        lv->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

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
void MyFrame::OnListItemSelected(wxListEvent& event) {
    wxListItem li = event.GetItem();
    exp_t *xp = (exp_t *)li.GetData();
    assert(xp != NULL);

    wxPropertyGrid *pg = (wxPropertyGrid *) wxWindow::FindWindowById(ID_EXPENSE_GRID);

    wxPGProperty *prop = pg->GetProperty("Description");
    assert(prop != NULL);
    prop->SetValue(wxVariant(wxString(xp->desc->s)));

    prop = pg->GetProperty("Amount");
    assert(prop != NULL);
    prop->SetValue(wxVariant(xp->amt));

    prop = pg->GetProperty("Category");
    assert(prop != NULL);
    prop->SetValue(wxVariant((int)xp->catid));

    prop = pg->GetProperty("Date");
    assert(prop != NULL);
    prop->SetValue(wxVariant(wxDateTime(date_time(xp->date))));
}
void MyFrame::OnListItemActivated(wxListEvent& event) {
}

void MyFrame::OnPropertyGridChanged(wxPropertyGridEvent& event) {
    wxPropertyGrid *pg = (wxPropertyGrid *) wxWindow::FindWindowById(ID_EXPENSE_GRID);
    wxPGProperty *prop = event.GetProperty();
    const wxString& name = prop->GetName();
    const wxString& label = prop->GetLabel();

    wxLogDebug("OnPropertyGridChanged() name: '%s' label: '%s'\n", name, label);

    pg->SetPropertyValue(prop, "changed!");

}

