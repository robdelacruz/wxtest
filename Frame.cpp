#include <stdio.h>

#include "wx/wx.h"
#include "wx/window.h"
#include "wx/choice.h"
#include "wx/listctrl.h"
#include "wx/splitter.h"
#include "wx/notebook.h"
#include "wx/filename.h"
#include "wx/propgrid/propgrid.h"
#include "wx/propgrid/advprops.h"
#include "wx/valnum.h"

#include "art/home.xpm"
#include "art/back.xpm"
#include "art/forward.xpm"
#include "art/up.xpm"
#include "art/down.xpm"

#include "App.h"
#include "Frame.h"
#include "EditExpenseDialog.h"
#include "SetupCategoriesDialog.h"
#include "db.h"

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_NEW, MyFrame::OnFileNew)
    EVT_MENU(wxID_OPEN, MyFrame::OnFileOpen)
    EVT_MENU(wxID_CLOSE, MyFrame::OnFileClose)
    EVT_MENU(wxID_EXIT, MyFrame::OnFileExit)

    EVT_MENU(ID_EXPENSE_NEW, MyFrame::OnExpenseNew)
    EVT_MENU(ID_EXPENSE_EDIT, MyFrame::OnExpenseEdit)
    EVT_MENU(ID_EXPENSE_DEL, MyFrame::OnExpenseDel)
    EVT_MENU(ID_EXPENSE_CATEGORIES, MyFrame::OnExpenseCategories)

    EVT_NOTEBOOK_PAGE_CHANGED(ID_NAV_NOTEBOOK, MyFrame::OnNavPageChanged)
    EVT_TEXT(ID_NAV_YEAR, MyFrame::OnNavYearChanged)
    EVT_BUTTON(ID_NAV_PREVYEAR, MyFrame::OnNavPrevYear)
    EVT_BUTTON(ID_NAV_NEXTYEAR, MyFrame::OnNavNextYear)
    EVT_LIST_ITEM_SELECTED(ID_NAV_MONTHSLIST, MyFrame::OnNavMonthSelected)
    EVT_LIST_ITEM_SELECTED(ID_NAV_YEARSLIST, MyFrame::OnNavYearSelected)

    EVT_BUTTON(ID_EXPENSES_PREV, MyFrame::OnPrevious)
    EVT_BUTTON(ID_EXPENSES_NEXT, MyFrame::OnNext)
    EVT_LIST_ITEM_SELECTED(ID_EXPENSES_LIST, MyFrame::OnListItemSelected)
    EVT_LIST_ITEM_DESELECTED(ID_EXPENSES_LIST, MyFrame::OnListItemDeselected)
    EVT_LIST_ITEM_ACTIVATED(ID_EXPENSES_LIST, MyFrame::OnListItemActivated)

    EVT_PG_CHANGED(ID_EXPENSE_GRID, MyFrame::OnPropertyGridChanged)
wxEND_EVENT_TABLE()

MyFrame::MyFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(640,480)) {
//    wxFont font = GetFont();

    setlocale(LC_NUMERIC, "");
    m_propgrid_xp = NULL;

    SetIcon(wxIcon(home_xpm));

//    font.SetPixelSize(wxSize(16,16));
//    font.SetPointSize(12);
//    SetFont(font);

    CreateMenuBar();
    CreateStatusBar(2);
    SetStatusText(wxT("Expense Buddy"));
    CreateControls();
    RefreshFrame();
    RefreshMenu();
    RefreshNav();
    RefreshExpenses(0, 0);
}

MyFrame::~MyFrame() {
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
        menu->AppendSeparator();
        menu->Append(ID_EXPENSE_CATEGORIES, wxT("&Categories...\tCtrl-T"), wxT("Setup Categories..."));
        mb->Append(menu, wxT("&Expense"));

        menu = new wxMenu;
        menu->Append(ID_VIEW_MONTHLY, "&Monthly", "Monthly View", wxITEM_CHECK);
        menu->Append(ID_VIEW_DAILY, "&Daily", "Daily View", wxITEM_CHECK);
        mb->Append(menu, "&View");
    }
    SetMenuBar(mb);
}

void MyFrame::CreateControls() {
    wxStaticText *st;
    wxPanel *pnlMain;
    wxSplitterWindow *split;
    wxWindow *nav;
    wxWindow *expview;
    wxBoxSizer *vbox;
    wxBoxSizer *vboxMain;

    DestroyChildren();
    SetSizer(NULL);

    st = new wxStaticText(this, ID_NO_OPEN_FILE, "No expense file open.", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
    pnlMain = new wxPanel(this, ID_MAIN_PANEL);

    split = new wxSplitterWindow(pnlMain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D);
    nav = CreateNav(split);
    expview = CreateExpensesView(split);
    split->SplitVertically(nav, expview);
    split->SetSashPosition(155);

    vboxMain = new wxBoxSizer(wxVERTICAL);
    vboxMain->Add(split, 1, wxEXPAND, 0);
    pnlMain->SetSizer(vboxMain);

    vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(st, 0, wxEXPAND|wxALL, 5);
    vbox->Add(pnlMain, 1, wxEXPAND, 0);
    this->SetSizer(vbox);

    wxWindow::FindWindowById(ID_EXPENSES_LIST)->SetFocus();
}

wxWindow* MyFrame::CreateNav(wxWindow *parent) {
    ExpenseContext *ctx = getContext();
    wxNotebook *nb;
    wxPanel *pnl1, *pnl2;
    wxTextCtrl *txtYear;
    wxIntegerValidator<int> vldYear(NULL, wxNUM_VAL_DEFAULT);
    wxBitmapButton *btnPrevYear, *btnNextYear;
    wxListView *lvMonths;
    wxListView *lvYears;
    wxListItem colAmount;
    wxBoxSizer *hbox;
    wxBoxSizer *vbox;
    date_t dt;
    char buf[20];

    nb = new wxNotebook(parent, ID_NAV_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxNB_TOP);

    pnl1 = new wxPanel(nb, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    pnl2 = new wxPanel(nb, wxID_ANY, wxDefaultPosition, wxDefaultSize);

    // pnl2: All years listview
    lvYears = new wxListView(pnl2, ID_NAV_YEARSLIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_NO_HEADER);
    lvYears->AppendColumn("Year");
    lvYears->AppendColumn("Amount");
    lvYears->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
    lvYears->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
    lvYears->GetColumn(1, colAmount);
    colAmount.SetAlign(wxLIST_FORMAT_RIGHT);
    lvYears->SetColumn(1, colAmount);

    // pnl1: Year selector with 12 months listview
    txtYear = new wxTextCtrl(pnl1, ID_NAV_YEAR, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, vldYear);
    btnPrevYear = new wxBitmapButton(pnl1, ID_NAV_PREVYEAR, wxBitmap(back_xpm), wxDefaultPosition, wxSize(-1, BTN_HEIGHT));
    btnNextYear = new wxBitmapButton(pnl1, ID_NAV_NEXTYEAR, wxBitmap(forward_xpm), wxDefaultPosition, wxSize(-1, BTN_HEIGHT));
    lvMonths = new wxListView(pnl1, ID_NAV_MONTHSLIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_NO_HEADER);
    lvMonths->AppendColumn("Month");
    lvMonths->AppendColumn("Amount");
    lvMonths->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
    lvMonths->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
    lvMonths->GetColumn(1, colAmount);
    colAmount.SetAlign(wxLIST_FORMAT_RIGHT);
    lvMonths->SetColumn(1, colAmount);

    // 2024, January, February, ... December
    dt = date_from_cal(ctx->year, 1, 1);
    lvMonths->InsertItem(0, wxString::Format("%d", ctx->year));
    for (int i=1; i <= 12; i++) {
        date_strftime(dt, "%B", buf, sizeof(buf));
        lvMonths->InsertItem(i, wxString::FromUTF8(buf));
        dt = date_next_month(dt);
    }

    hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(btnPrevYear, 0, wxEXPAND, 0);
    hbox->Add(txtYear, 0, wxEXPAND, 0);
    hbox->Add(btnNextYear, 0, wxEXPAND, 0);

    vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(hbox, 0, wxEXPAND, 0);
    vbox->AddSpacer(5);
    vbox->Add(lvMonths, 1, wxEXPAND, 0);
    pnl1->SetSizer(vbox);

    vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(lvYears, 1, wxEXPAND, 0);
    pnl2->SetSizer(vbox);

    nb->AddPage(pnl1, "Monthly", true);
    nb->AddPage(pnl2, "Yearly", false);

    return nb;
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
    wxStaticText *stHeading;
    wxBitmapButton *btnPrev, *btnNext;
    wxStaticText *lblFilter;
    wxTextCtrl *txtFilter;
    wxListItem colAmount;
    wxBoxSizer *vbox;
    wxBoxSizer *hbox;

    pnl = new wxPanel(parent, ID_EXPENSES_PANEL, wxDefaultPosition, wxSize(-1,200));

    stHeading = new wxStaticText(pnl, ID_EXPENSES_HEADING, "");
    btnPrev = new wxBitmapButton(pnl, ID_EXPENSES_PREV, wxBitmap(back_xpm), wxDefaultPosition, wxSize(-1, BTN_HEIGHT));
    btnNext = new wxBitmapButton(pnl, ID_EXPENSES_NEXT, wxBitmap(forward_xpm), wxDefaultPosition, wxSize(-1, BTN_HEIGHT));
    lblFilter = new wxStaticText(pnl, wxID_ANY, "Filter");
    txtFilter = new wxTextCtrl(pnl, ID_EXPENSES_FILTERTEXT, wxT(""), wxDefaultPosition, wxSize(300, BTN_HEIGHT));

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

    hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(btnPrev, 0, wxALIGN_CENTER, 0);
    hbox->AddSpacer(5);
    hbox->Add(stHeading, 0, wxALIGN_CENTER, 0);
    hbox->AddSpacer(5);
    hbox->Add(btnNext, 0, wxALIGN_CENTER, 0);
    hbox->AddSpacer(5);
    hbox->Add(lblFilter, 0, wxALIGN_CENTER, 0);
    hbox->AddSpacer(2);
    hbox->Add(txtFilter, 1, wxEXPAND, 0);

    vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(hbox, 0, wxEXPAND, 0);
    vbox->Add(lv, 1, wxEXPAND, 0);
    pnl->SetSizer(vbox);

    return pnl;
}
wxWindow* MyFrame::CreateExpensePropGrid(wxWindow *parent) {
    wxPropertyGrid *pg;
    wxFloatProperty *floatprop;
    wxDateProperty *dateprop;
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
    floatprop = new wxFloatProperty("Amount", wxPG_LABEL);
    floatprop->SetAttribute(wxPG_FLOAT_PRECISION, 2);
    pg->Append(floatprop);
    pg->Append(new wxEnumProperty("Category", wxPG_LABEL, cats, idcats));
    dateprop = new wxDateProperty("Date", wxPG_LABEL);
    dateprop->SetAttribute(wxPG_DATE_PICKER_STYLE, wxDP_DEFAULT|wxDP_SHOWCENTURY|wxDP_ALLOWNONE);
    pg->Append(dateprop);

    return pg;
}

void MyFrame::RefreshFrame() {
    ExpenseContext *ctx = getContext();
    wxWindow *stNoOpenFile;
    wxWindow *pnlMain;
    wxString filename, ext;

    stNoOpenFile = wxWindow::FindWindowById(ID_NO_OPEN_FILE);
    pnlMain = wxWindow::FindWindowById(ID_MAIN_PANEL);

    // No open expense file
    if (!ctx_is_open_expfile(ctx)) {
        SetTitle(wxT("Expense Buddy"));
        stNoOpenFile->Show(true);
        pnlMain->Show(false);
        Layout();
        return;
    }

    // Expense file opened
    wxFileName::SplitPath(wxString::FromUTF8(ctx->expfile->s), NULL, NULL, &filename, &ext);
    SetTitle(wxString::Format("Expense Buddy - [%s.%s]", filename, ext));
    stNoOpenFile->Show(false);
    pnlMain->Show(true);
    Layout();
}

void MyFrame::RefreshMenu() {
    ExpenseContext *ctx = getContext();
    wxMenuBar *mb = GetMenuBar();

    if (!ctx_is_open_expfile(ctx))
        return;

    if (ctx->xps->len > 0) {
        mb->Enable(ID_EXPENSE_EDIT, true);
        mb->Enable(ID_EXPENSE_DEL, true);
    } else {
        mb->Enable(ID_EXPENSE_EDIT, false);
        mb->Enable(ID_EXPENSE_DEL, false);
    }
}
void MyFrame::RefreshNav() {
    ExpenseContext *ctx = getContext();
    wxNotebook *nb = (wxNotebook *) wxWindow::FindWindowById(ID_NAV_NOTEBOOK);
    char buf[24];

    // ctx->subtotals array holds subtotals for all years and months in date desc order
    // It contains all years from highest to lowest that have expenses.
    //
    // st.year: 2020, st.month: 0  st.total contains amt subtotal for entire 2020
    // st.year: 2020, st.month: 1  st.total contains amt subtotal for 2020 January
    // st.year: 2020, st.month: 2  st.total contains amt subtotal for 2020 February
    // st.year: 2020, st.month: 12 st.total contains amt subtotal for 2020 December
    //
    // If the year/month has no expenses, subtotal will be 0.
    // Ex. if there are no expenses for 2019, but there are expenses for 2018,
    // then the entire year/month records for 2019/month will be zero.

    if (nb->GetSelection() == 0) {
        // Refresh monthly nav
        wxTextCtrl *txtYear = (wxTextCtrl *) wxWindow::FindWindowById(ID_NAV_YEAR);
        wxListView *lvMonths = (wxListView *) wxWindow::FindWindowById(ID_NAV_MONTHSLIST);
        int istart = -1;

        txtYear->ChangeValue(wxString::Format("%d", ctx->year));
        lvMonths->Select(ctx->month);

        for (size_t i=0; i < ctx->subtotals->len; i++) {
            // Find subtotal for ctx->year
            subtotal_t *st = (subtotal_t *) ctx->subtotals->items[i];
            if (st->year == ctx->year) {
                assert(st->month == 0);  // Year subtotal should always appear first
                istart = i;              // subtotal records for ctx->year starts at istart
                break;
            }
        }
        if (istart == -1) {
            lvMonths->SetItem(0, 0, wxString::Format("%d", ctx->year));
            snprintf(buf, sizeof(buf), "%'9.2f", 0.0);
            for (int i=0; i <= 12; i++)
                lvMonths->SetItem(i, 1, buf);
        } else {
            // There should always be 13 records available from istart.
            assert((size_t)istart+12 < ctx->subtotals->len);

            lvMonths->SetItem(0, 0, wxString::Format("%d", ctx->year));

            // There should be 13 records from istart index without gaps.
            // index 0: subtotal for entire year
            // indexes 1-12: subtotals for Jan-Dec
            for (int i=0; i <= 12; i++) {
                subtotal_t *st;

                if (istart+i >= (int)ctx->subtotals->len)
                    break;
                st = (subtotal_t *) ctx->subtotals->items[istart+i];
                assert(i == st->month);

                snprintf(buf, sizeof(buf), "%'9.2f", st->total);
                lvMonths->SetItem(i, 1, buf);
            }
        }
    } else {
        // Refresh yearly nav
        int nitem;
        wxListView *lvYears = (wxListView *) wxWindow::FindWindowById(ID_NAV_YEARSLIST);
        lvYears->DeleteAllItems();

        nitem = 0;
        for (size_t i=0; i < ctx->subtotals->len; i++) {
            subtotal_t *st = (subtotal_t *) ctx->subtotals->items[i];
            if (st->month != 0)
                continue;

            lvYears->InsertItem(nitem, wxString::Format("%d", st->year));
            snprintf(buf, sizeof(buf), "%'9.2f", st->total);
            lvYears->SetItem(nitem, 1, buf);
            lvYears->SetItemData(nitem, st->year);

            if (st->year == ctx->year)
                lvYears->Select(nitem);
            nitem++;
        }
    }
}
void MyFrame::RefreshExpenses(uint64_t sel_expid, long sel_row) {
    // RefreshExpenseGrid() needs to be called before RefreshExpenseList() because
    // RefreshExpenseList() selects the row, which activates OnListItemSelected(),
    // which calls RefreshExpenseGrid(xp).
    RefreshExpenseGrid(NULL);
    RefreshExpensesList(sel_expid, sel_row);

    wxWindow::FindWindowById(ID_EXPENSES_PANEL)->Layout();
}

// To select row number: RefreshExpenseList(0, rownum)
// To select expense id: RefreshExpenseList(expid, 0)
void MyFrame::RefreshExpensesList(uint64_t sel_expid, long sel_row) {
    ExpenseContext *ctx = getContext();
    wxListView *lv;
    wxStaticText *st;
    exp_t *xp;
    char buf[24];

    st = (wxStaticText *) wxWindow::FindWindowById(ID_EXPENSES_HEADING, this);
    if (ctx->month == 0)
        date_strftime(date_from_cal(ctx->year, 1, 1), "%Y", buf, sizeof(buf));
    else
        date_strftime(date_from_cal(ctx->year, ctx->month, 1), "%b %Y", buf, sizeof(buf));
    st->SetLabel(wxString::FromUTF8(buf));

    lv = (wxListView *) wxWindow::FindWindowById(ID_EXPENSES_LIST, this);
    lv->DeleteAllItems();

    for (size_t i=0; i < ctx->xps->len; i++) {
        xp = (exp_t *) ctx->xps->items[i];

        date_strftime(xp->date, "%m-%d", buf, sizeof(buf));
        lv->InsertItem(i, buf);
        lv->SetItem(i, 1, xp->desc->s);
        snprintf(buf, sizeof(buf), "%'9.2f", xp->amt);
        lv->SetItem(i, 2, buf);
        lv->SetItem(i, 3, xp->catname->s);

        lv->SetItemPtrData(i, (wxUIntPtr)xp);

        if (sel_expid != 0 && sel_expid == xp->expid)
            sel_row = i;
    }
    // Restore previous row selection.
    if (sel_row >= 0 && (size_t) sel_row < ctx->xps->len) {
        lv->SetItemState(sel_row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        lv->EnsureVisible(sel_row);
    }
}
void MyFrame::RefreshSingleExpenseInList(exp_t *xp) {
    ExpenseContext *ctx = getContext();
    wxListView *lv;
    char buf[24];

    lv = (wxListView *) wxWindow::FindWindowById(ID_EXPENSES_LIST, this);
    for (size_t i=0; i < ctx->xps->len; i++) {
        exp_t *listxp = (exp_t *) ctx->xps->items[i];
        if (listxp->expid == xp->expid) {
            date_strftime(xp->date, "%m-%d", buf, sizeof(buf));
            lv->SetItem(i, 0, buf);
            lv->SetItem(i, 1, xp->desc->s);
            snprintf(buf, sizeof(buf), "%9.2f", xp->amt);
            lv->SetItem(i, 2, buf);
            lv->SetItem(i, 3, xp->catname->s);
            break;
        }
    }
}
void MyFrame::RefreshExpenseGrid(exp_t *xp) {
    wxPropertyGrid *pg = (wxPropertyGrid *) wxWindow::FindWindowById(ID_EXPENSE_GRID);

    m_propgrid_xp = xp;

    if (xp == NULL) {
        pg->GetProperty("Description")->SetValue(wxVariant(wxString("")));
        pg->GetProperty("Amount")->SetValue(wxVariant(0.0));
        pg->GetProperty("Category")->SetValue(wxVariant(0));
        pg->GetProperty("Date")->SetValue(wxVariant(wxDateTime()));
        pg->Show(false);
        return;
    }
    pg->GetProperty("Description")->SetValue(wxVariant(wxString(xp->desc->s)));
    pg->GetProperty("Amount")->SetValue(wxVariant(xp->amt));
    pg->GetProperty("Category")->SetValue(wxVariant((int)xp->catid));
    pg->GetProperty("Date")->SetValue(wxVariant(wxDateTime(xp->date)));
    pg->Show(true);
}

void MyFrame::EditExpense(exp_t *xp) {
    ExpenseContext *ctx = getContext();
    int xpyear, xpmonth, xpday;
    uint64_t expid;

    EditExpenseDialog dlg(this, xp);
    if (dlg.ShowModal() == wxID_OK) {
        if (xp->expid == 0)
            db_add_exp(ctx->expfiledb, xp);
        else
            db_edit_exp(ctx->expfiledb, xp);

        date_to_cal(xp->date, &xpyear, &xpmonth, &xpday);
        ctx_set_date(ctx, xpyear, xpmonth, xpday);
        expid = xp->expid;

        ctx_refresh_expenses(ctx);
        ctx_refresh_subtotals_year_month(ctx, xpyear, xpmonth);
        ctx_refresh_cattotals(ctx);

        RefreshMenu();
        RefreshNav();
        RefreshExpenses(expid, 0);
    }
}

static wxString fileErrorString(wxString file, int errnum) {
    wxString errstr;
    errstr.Printf("%s: %s", exp_strerror(errnum), file);
    return errstr;
}

void MyFrame::OnFileNew(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    char expfile[1024];
    int z;

    wxFileDialog dlg(this, wxT("New Expense File"), wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_SAVE, wxDefaultPosition, wxDefaultSize);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;

    strncpy(expfile, dlg.GetPath().mb_str(), sizeof(expfile)-1);
    z = ctx_create_expense_file(ctx, expfile);
    if (z != 0) {
        wxMessageDialog msgdlg(NULL, fileErrorString(dlg.GetPath(), z), wxT("Error occured"), wxOK | wxICON_ERROR);
        msgdlg.ShowModal();
        return;
    }

    CreateMenuBar();
    RefreshFrame();
    RefreshMenu();
    RefreshNav();
    RefreshExpenses(0, 0);
}
void MyFrame::OnFileOpen(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    char expfile[1024];
    int z;

    wxFileDialog dlg(this, wxT("Open Expense File"), wxEmptyString, wxEmptyString, wxT("*.db"), wxFD_OPEN, wxDefaultPosition, wxDefaultSize);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;

    strncpy(expfile, dlg.GetPath().mb_str(), sizeof(expfile)-1);
    z = ctx_open_expense_file(ctx, expfile);
    if (z != 0) {
        wxMessageDialog msgdlg(NULL, fileErrorString(dlg.GetPath(), z), wxT("Error occured"), wxOK | wxICON_ERROR);
        msgdlg.ShowModal();
        return;
    }

    CreateMenuBar();
    RefreshFrame();
    RefreshMenu();
    RefreshNav();
    RefreshExpenses(0, 0);
}
void MyFrame::OnFileClose(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    ctx_close(ctx);

    CreateMenuBar();
    RefreshFrame();
    RefreshMenu();
    RefreshNav();
    RefreshExpenses(0, 0);
}
void MyFrame::OnFileExit(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    ctx_close(ctx);
    Close(true);
}

void MyFrame::OnExpenseNew(wxCommandEvent& event) {
    exp_t *xp = exp_new();
    EditExpense(xp);
    exp_free(xp);
}
void MyFrame::OnExpenseEdit(wxCommandEvent& event) {
    wxListView *lv = (wxListView *) wxWindow::FindWindowById(ID_EXPENSES_LIST);
    long lsel = lv->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (lsel == -1)
        return;

    exp_t *xp = (exp_t *) lv->GetItemData(lsel);
    assert(xp != NULL);
    EditExpense(xp);
}
void MyFrame::OnExpenseDel(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    int xpyear, xpmonth;
    wxListView *lv = (wxListView *) wxWindow::FindWindowById(ID_EXPENSES_LIST);
    long lsel = lv->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (lsel == -1)
        return;

    exp_t *xp = (exp_t *) lv->GetItemData(lsel);
    assert(xp != NULL);

    wxMessageDialog dlg(this, wxString::Format("Delete '%s'?", wxString::FromUTF8(xp->desc->s)), "Confirm Delete", wxYES_NO | wxNO_DEFAULT); 
    if (dlg.ShowModal() != wxID_YES)
        return;
    db_del_exp(ctx->expfiledb, xp->expid);
    date_to_cal(xp->date, &xpyear, &xpmonth, NULL);

    ctx_refresh_expenses(ctx);
    ctx_refresh_subtotals_year_month(ctx, xpyear, xpmonth);
    ctx_refresh_cattotals(ctx);

    if ((size_t) lsel > ctx->xps->len-1)
        lsel = ctx->xps->len-1;
    RefreshNav();
    RefreshExpenses(0, lsel);
}
void MyFrame::OnExpenseCategories(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    wxListView *lv = (wxListView *) wxWindow::FindWindowById(ID_EXPENSES_LIST);
    long lsel = lv->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (lsel == -1)
        lsel = 0;

    SetupCategoriesDialog dlg(this);
    dlg.ShowModal();

    ctx_refresh_subtotals(ctx);
    ctx_refresh_categories(ctx);
    ctx_refresh_expenses(ctx);
    ctx_refresh_cattotals(ctx);
    RefreshExpenses(0, lsel);
}

void MyFrame::OnPrevious(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    if (ctx->month == 0)
        ctx->year--;
    else
        ctx_set_date_previous_month(ctx);

    ctx_refresh_expenses(ctx);
    ctx_refresh_cattotals(ctx);
    RefreshMenu();
    RefreshNav();
    RefreshExpenses(0, 0);
}
void MyFrame::OnNext(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    if (ctx->month == 0)
        ctx->year++;
    else
        ctx_set_date_next_month(ctx);

    ctx_refresh_expenses(ctx);
    ctx_refresh_cattotals(ctx);
    RefreshMenu();
    RefreshNav();
    RefreshExpenses(0, 0);
}
void MyFrame::OnListItemSelected(wxListEvent& event) {
    wxMenuBar *mb = GetMenuBar();
    exp_t *xp = (exp_t *)event.GetItem().GetData();
    assert(xp != NULL);

    mb->Enable(ID_EXPENSE_EDIT, true);
    mb->Enable(ID_EXPENSE_DEL, true);
    RefreshExpenseGrid(xp);
}
void MyFrame::OnListItemDeselected(wxListEvent& event) {
    wxMenuBar *mb = GetMenuBar();

    mb->Enable(ID_EXPENSE_EDIT, false);
    mb->Enable(ID_EXPENSE_DEL, false);
    RefreshExpenseGrid(NULL);
}
void MyFrame::OnListItemActivated(wxListEvent& event) {
    wxListItem li = event.GetItem();
    exp_t *xp = (exp_t *)li.GetData();
    assert(xp != NULL);

    EditExpense(xp);
}

//    wxPGProperty *prop = event.GetProperty();
//    const wxString& name = prop->GetName();
//    wxVariant v = prop->GetValue();
//    pg->SetPropertyValue(prop, "changed!");

void MyFrame::OnPropertyGridChanged(wxPropertyGridEvent& event) {
    ExpenseContext *ctx = getContext();
    wxPropertyGrid *pg;
    wxPGProperty *propDesc, *propAmount, *propCat, *propDate;
    wxString desc;
    double amt;
    uint64_t catid;
    wxDateTime date;
    exp_t *xp;

    if (m_propgrid_xp == NULL)
        return;
    xp = m_propgrid_xp;

    pg = (wxPropertyGrid *) wxWindow::FindWindowById(ID_EXPENSE_GRID);
    propDesc = pg->GetProperty("Description");
    propAmount = pg->GetProperty("Amount");
    propCat = pg->GetProperty("Category");
    propDate = pg->GetProperty("Date");

    desc = propDesc->GetValue().GetString();
    amt = propAmount->GetValue().GetDouble();
    catid = (uint64_t) propCat->GetValue().GetLong();
    date = propDate->GetValue().GetDateTime();

    str_assign(xp->desc, desc.mb_str());
    xp->amt = amt;
    xp->catid = catid;
    xp->date = date.GetTicks();

    db_edit_exp(ctx->expfiledb, xp);
    // update xp->catname in case xp->catid was changed
    db_find_exp_by_id(ctx->expfiledb, xp->expid, xp);

    RefreshNav();
    RefreshSingleExpenseInList(xp);
}

void MyFrame::OnNavPageChanged(wxBookCtrlEvent& event) {
    RefreshNav();
}

static int GetNavYear() {
    ExpenseContext *ctx = getContext();
    wxTextCtrl *txtYear = (wxTextCtrl *) wxWindow::FindWindowById(ID_NAV_YEAR);
    wxString strYear = txtYear->GetValue().Trim().Truncate(4);
    int year;

    if (strYear.Length() < 4)
        return ctx->year;
    if (!strYear.ToInt(&year))
        return ctx->year;
    if (year < 1970 || year > 2100)
        return ctx->year;

    if (year < 1970)
        year = 1970;
    if (year > 2100)
        year = 2100;

    return year;
}

void MyFrame::OnNavYearChanged(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    int year = GetNavYear();

    if (year == ctx->year)
        return;

    ctx_set_date(ctx, year, 0, 0);
    ctx_refresh_expenses(ctx);
    ctx_refresh_cattotals(ctx);
    RefreshMenu();
    RefreshNav();
    RefreshExpenses(0, 0);
}
void MyFrame::OnNavPrevYear(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    int year = GetNavYear();

    ctx_set_date(ctx, year-1, 0, 0);
    ctx_refresh_expenses(ctx);
    ctx_refresh_cattotals(ctx);
    RefreshMenu();
    RefreshNav();
    RefreshExpenses(0, 0);
}
void MyFrame::OnNavNextYear(wxCommandEvent& event) {
    ExpenseContext *ctx = getContext();
    int year = GetNavYear();

    ctx_set_date(ctx, year+1, 0, 0);
    ctx_refresh_expenses(ctx);
    ctx_refresh_cattotals(ctx);
    RefreshMenu();
    RefreshNav();
    RefreshExpenses(0, 0);
}
void MyFrame::OnNavMonthSelected(wxListEvent& event) {
    ExpenseContext *ctx = getContext();
    int month = event.GetIndex();

    ctx_set_date(ctx, 0, month, 0);
    ctx_refresh_expenses(ctx);
    ctx_refresh_cattotals(ctx);
    RefreshMenu();
    RefreshExpenses(0, 0);

    for (size_t i=0; i < ctx->cattotals->len; i++) {
        cattotal_t *ct = (cattotal_t *) ctx->cattotals->items[i];
        printf("catname: '%s' total: %'9.2f numxps: %d\n", ct->catname->s, ct->total, ct->numxps);
    }
}
void MyFrame::OnNavYearSelected(wxListEvent& event) {
    ExpenseContext *ctx = getContext();
    int year = (int) event.GetItem().GetData();

    ctx_set_date(ctx, year, 0, 0);
    ctx_refresh_expenses(ctx);
    ctx_refresh_cattotals(ctx);
    RefreshMenu();
    RefreshExpenses(0, 0);
}

