#include "wx/wx.h"
#include "wx/listctrl.h"
#include "frame.h"

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_EXIT, MyFrame::OnQuit)
END_EVENT_TABLE()

void MyFrame::OnQuit(wxCommandEvent& event) {
    Close();
}

MyFrame::MyFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title) {
    wxMenuBar *menubar;
    wxMenu *fileMenu;
    wxListView *lv;
    wxBoxSizer *vbox;

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

    lv->InsertItem(0, "03-20");
    lv->SetItem(0, 1, "starbucks");
    lv->SetItem(0, 2, "105.0");
    lv->SetItem(0, 3, "coffee");

    vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(lv, 1, wxEXPAND, 0);
    SetSizer(vbox);
    vbox->Fit(this);
//    vbox->SetSizeHints(this);
}


