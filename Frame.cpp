#include <stdio.h>

#include "wx/wx.h"
#include "wx/window.h"
#include "wx/listctrl.h"
#include "wx/splitter.h"
#include "wx/spinctrl.h"
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

    ID_NO_OPEN_FILE,
    ID_MAIN_SPLIT,

    ID_NAV_YEAR_SPIN,
    ID_NAV_MONTH_LIST,

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

    EVT_SPINCTRL(ID_NAV_YEAR_SPIN, MyFrame::OnNavYear)
    EVT_LISTBOX(ID_NAV_MONTH_LIST, MyFrame::OnNavMonth)

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
    SetStatusText(wxT("Expense Buddy"));
    CreateControls();
    ShowControls();
    RefreshNav();
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
    wxStaticText *st;
    wxSplitterWindow *split;
    wxWindow *pnlNav, *pnlView;
    wxBoxSizer *vbox;

    DestroyChildren();
    SetSizer(NULL);

    st = new wxStaticText(this, ID_NO_OPEN_FILE, "No expense file open.", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
    split = new wxSplitterWindow(this, ID_MAIN_SPLIT, wxDefaultPosition, wxDefaultSize, wxSP_3D);
    pnlNav = CreateExpensesNav(split);
    pnlView = CreateExpensesView(split);
    split->SplitVertically(pnlNav, pnlView);
    split->SetSashPosition(150);

    vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(st, 0, wxEXPAND|wxALL, 5);
    vbox->Add(split, 1, wxEXPAND, 0);
    this->SetSizer(vbox);

}

wxWindow* MyFrame::CreateExpensesNav(wxWindow *parent) {
    wxPanel *pnl;
    wxSpinCtrl *spinYear;
    wxListBox *lbMonth;
    wxBoxSizer *vbox;

    pnl = new wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(100,-1));
    spinYear = new wxSpinCtrl(pnl, ID_NAV_YEAR_SPIN, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1900, 2100);
    lbMonth = new wxListBox(pnl, ID_NAV_MONTH_LIST, wxDefaultPosition, wxDefaultSize);

    // 12 Months of the year in listbox
    //for (wxDateTime::Month month = wxDateTime::Jan; month <= wxDateTime::Dec; month += 1) {
    for (int month = wxDateTime::Jan; month <= wxDateTime::Dec; month++) {
        lbMonth->Append(wxDateTime::GetMonthName((wxDateTime::Month)month, wxDateTime::Name_Full));
    }

    vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(spinYear, 0, wxEXPAND|wxALL, 5);
    vbox->Add(lbMonth, 1, wxEXPAND, 0);
    pnl->SetSizer(vbox);

    return pnl;
}

wxWindow* MyFrame::CreateExpensesView(wxWindow *parent) {
    wxSplitterWindow *split;
    wxWindow *explist;
    wxWindow *pg;

    split = new wxSplitterWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D);
    explist = CreateExpensesList(split);
    pg = CreateExpensePropGrid(split);
    split->SplitHorizontally(explist, pg);
    split->SetSashPosition(300);

    return split;
}

wxWindow* MyFrame::CreateExpensesList(wxWindow *parent) {
    wxPanel *pnl;
    wxListView *lv;
    wxBitmapButton *btnPrev, *btnNext;
    wxTextCtrl *txtFilter;
    wxBoxSizer *vbox;
    wxBoxSizer *hbox;
    wxListItem colAmount;

    pnl = new wxPanel(parent, ID_EXPENSES_PANEL, wxDefaultPosition, wxSize(-1,200));

    lv = new wxListView(pnl, ID_EXPENSES_LIST);
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

wxWindow* MyFrame::CreateExpensePropGrid(wxWindow *parent) {
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

void MyFrame::ShowControls() {
    ExpenseContext *ctx = getContext();
    wxWindow *stNoOpenFile, *splitMain;
    wxString title, filename, ext;

    stNoOpenFile = wxWindow::FindWindowById(ID_NO_OPEN_FILE);
    splitMain = wxWindow::FindWindowById(ID_MAIN_SPLIT);

    // No open expense file
    if (!ctx_is_open_expfile(ctx)) {
        SetTitle(wxT("Expense Buddy"));
        stNoOpenFile->Show(true);
        splitMain->Show(false);
        return;
    }

    // Expense file opened
    wxFileName::SplitPath(wxString::FromUTF8(ctx->expfile->s), NULL, NULL, &filename, &ext);
    title.Printf("Expense Buddy - [%s.%s]", filename, ext);
    SetTitle(title);
    stNoOpenFile->Show(false);
    splitMain->Show(true);
}

void MyFrame::RefreshNav() {
    ExpenseContext *ctx = getContext();
    wxSpinCtrl *spinYear = (wxSpinCtrl *) wxWindow::FindWindowById(ID_NAV_YEAR_SPIN);
    wxListBox *lbMonth = (wxListBox *) wxWindow::FindWindowById(ID_NAV_MONTH_LIST);
    wxString strMonthNameItem;
    double sumamt;

    spinYear->SetValue(date_year(ctx->dt));
    lbMonth->SetSelection(date_month(ctx->dt)-1);

    int month = wxDateTime::Jan;
    for (int i=0; i < 12; i++) {
        wxString monthName = wxDateTime::GetMonthName((wxDateTime::Month)month, wxDateTime::Name_Full);
        ctx_expenses_sum_amount(ctx, date_year(ctx->dt), i+1, &sumamt);
        strMonthNameItem.Printf("%s (%.2f)", monthName, sumamt);
        lbMonth->SetString(i, strMonthNameItem);
        month++;
    }
}

void MyFrame::RefreshExpenses() {
    ExpenseContext *ctx = getContext();
    wxStaticText *st;
    char buf[24];

    st = (wxStaticText *) wxWindow::FindWindowById(ID_EXPENSES_HEADING, this);
    date_strftime(ctx->dt, "%b %Y", buf, sizeof(buf));
    st->SetLabel(wxString::FromUTF8(buf));

    RefreshExpenseList();
    RefreshExpenseGrid();

    wxWindow::FindWindowById(ID_EXPENSES_PANEL)->Layout();
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
    CreateControls();
    ShowControls();
    RefreshNav();
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
    CreateControls();
    ShowControls();
    RefreshNav();
    RefreshExpenses();
}
void MyFrame::OnFileClose(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    ctx_close(ctx);

    CreateMenuBar();
    ShowControls();
    RefreshNav();
    RefreshExpenses();
}
void MyFrame::OnFileExit(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    ctx_close(ctx);
    Close();
}

void MyFrame::OnPrevMonth(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    ctx_refresh_expenses_prev_month(ctx);
    RefreshNav();
    RefreshExpenses();
}
void MyFrame::OnNextMonth(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    ctx_refresh_expenses_next_month(ctx);
    RefreshNav();
    RefreshExpenses();
}
void MyFrame::OnListItemSelected(wxListEvent& event) {
    ExpenseContext *ctx = getContext();
    wxListItem li = event.GetItem();
    exp_t *xp = (exp_t *)li.GetData();
    assert(xp != NULL);

    ctx_select_expense(ctx, xp);
    RefreshExpenseGrid();
}

void MyFrame::ClearExpenseList() {
    wxListView *lv = (wxListView *) wxWindow::FindWindowById(ID_EXPENSES_LIST, this);
    lv->DeleteAllItems();
}
void MyFrame::RefreshExpenseList() {
    ExpenseContext *ctx = getContext();
    wxListView *lv;
    exp_t *xp;
    char buf[24];

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

        lv->SetItemPtrData(i, (wxUIntPtr)xp);
    }
    // Select first row
    if (ctx->xps->len > 0)
        lv->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void MyFrame::ClearExpenseGrid() {
    wxPropertyGrid *pg = (wxPropertyGrid *) wxWindow::FindWindowById(ID_EXPENSE_GRID);

    pg->GetProperty("Description")->SetValue(wxVariant(wxString("")));
    pg->GetProperty("Amount")->SetValue(wxVariant(0.0));
    pg->GetProperty("Category")->SetValue(wxVariant(0));
    //pg->GetProperty("Date")->SetValue(wxVariant(wxDateTime::Now()));
    pg->GetProperty("Date")->SetValue(wxVariant(wxDateTime()));
}
void MyFrame::RefreshExpenseGrid() {
    ExpenseContext *ctx = getContext();
    wxPropertyGrid *pg = (wxPropertyGrid *) wxWindow::FindWindowById(ID_EXPENSE_GRID);
    exp_t *xp = ctx_get_selected_expense(ctx);

    if (xp == NULL) {
        ClearExpenseGrid();
        pg->Show(false);
        return;
    }
    pg->GetProperty("Description")->SetValue(wxVariant(wxString(xp->desc->s)));
    pg->GetProperty("Amount")->SetValue(wxVariant(xp->amt));
    pg->GetProperty("Category")->SetValue(wxVariant((int)xp->catid));
    pg->GetProperty("Date")->SetValue(wxVariant(wxDateTime(date_time(xp->date))));
    pg->Show(true);
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

void MyFrame::OnNavYear(wxSpinEvent& event) {
    ExpenseContext *ctx = getContext();
    int year = event.GetPosition();

    ctx_refresh_expenses_year(ctx, year);
    RefreshNav();
    RefreshExpenses();
}

void MyFrame::OnNavMonth(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    int month = event.GetSelection()+1;

    ctx_refresh_expenses_month(ctx, month);
    RefreshExpenses();
}

