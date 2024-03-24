#include "all.h"
#include <stdio.h>

#include "wx/listctrl.h"
#include "art/home.xpm"
#include "art/back.xpm"
#include "art/forward.xpm"

#define ID_PREV_MONTH 101
#define ID_NEXT_MONTH 102

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_EXIT, MyFrame::OnQuit)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, MyFrame::OnListItemActivated)
END_EVENT_TABLE()

MyFrame::MyFrame(const wxString& title, sqlite3 *db, char *dbfile) : wxFrame(NULL, wxID_ANY, title) {
    wxMenuBar *menubar;
    wxMenu *fileMenu;
    wxListView *lv;
    wxBitmapButton *btnPrevMonth, *btnNextMonth;
    wxBoxSizer *vbox;
    wxBoxSizer *hbox;
    array_t *xps;
    array_t *cats;
    char bufdate[ISO_DATE_LEN+1];
    char bufamt[12];

    m_db = db;
    m_dbfile = str_new(0);
    if (dbfile)
        str_assign(m_dbfile, dbfile);
    xps = array_new(0);
    cats = array_new(0);

    SetIcon(wxIcon(home_xpm));

    fileMenu = new wxMenu;
    fileMenu->Append(wxID_EXIT, wxT("E&xit\tCtrl-Q"), wxT("Quit program"));

    menubar = new wxMenuBar();
    menubar->Append(fileMenu, wxT("&File"));
    SetMenuBar(menubar);

    CreateStatusBar(2);
    SetStatusText(wxT("Welcome to wxWidgets."));

    //lv = new wxListView(this, wxID_ANY, wxDefaultPosition, wxSize(250,200));
    lv = new wxListView(this, wxID_ANY);
    lv->AppendColumn("Date");
    lv->AppendColumn("Description");
    lv->AppendColumn("Amount");
    lv->AppendColumn("Category");

    db_select_exp(m_db, xps);
    db_select_cat(m_db, cats);

    for (uint i=0; i < xps->len; i++) {
        exp_t *xp = (exp_t*)xps->items[i];
        date_strftime(xp->date, (char*)"%m-%d", bufdate, sizeof(bufdate));
        lv->InsertItem(i, bufdate);
        lv->SetItem(i, 1, xp->desc->s);
        snprintf(bufamt, sizeof(bufamt), "%9.2f", xp->amt);
        lv->SetItem(i, 2, bufamt);
        lv->SetItem(i, 3, xp->catname->s);
    }
    for (int col=0; col <= 3; col++)
        lv->SetColumnWidth(col, wxLIST_AUTOSIZE_USEHEADER);

    lv->SetColumnWidth(1, 200);

    btnPrevMonth = new wxBitmapButton(this, wxID_ANY, wxBitmap(back_xpm));
    btnNextMonth = new wxBitmapButton(this, wxID_ANY, wxBitmap(forward_xpm));
    hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(btnPrevMonth, 0, wxEXPAND, 0);
    hbox->AddStretchSpacer();
    hbox->Add(new wxStaticText(this, wxID_ANY, wxT("March, 2024")), 0, wxALIGN_CENTER_VERTICAL, 0);
    hbox->AddStretchSpacer();
    hbox->Add(btnNextMonth, 0, wxEXPAND, 0);

    vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(hbox, 0, wxEXPAND|wxTOP|wxBOTTOM, 5);
    vbox->Add(lv, 1, wxEXPAND, 0);
    SetSizer(vbox);
    vbox->Fit(this);
//    vbox->SetSizeHints(this);
}

void MyFrame::OnQuit(wxCommandEvent& event) {
    Close();
}

void MyFrame::OnListItemActivated(wxListEvent& event) {
    long i = event.GetIndex();
    printf("list activated GetIndex(): %ld\n", i);
    wxListItem item = event.GetItem();
    wxString s = event.GetText();
    printf("on list selected: '%s'\n", s.mb_str().data());
}




