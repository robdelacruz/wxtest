#include <stdio.h>
#include "wx/wx.h"
#include "wx/listctrl.h"
#include "frame.h"
#include "art/home.xpm"
#include "art/back.xpm"
#include "art/forward.xpm"

#define ID_PREV_MONTH 101
#define ID_NEXT_MONTH 102

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_EXIT, MyFrame::OnQuit)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, MyFrame::OnListItemActivated)
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

    for (int i=0; i < 20; i++) {
        lv->InsertItem(i, "03-20");
        lv->SetItem(i, 1, "starbucks");
        lv->SetItem(i, 2, "105.0");
        lv->SetItem(i, 3, "coffee");
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




