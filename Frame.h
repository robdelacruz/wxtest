#ifndef FRAME_H
#define FRAME_H

#include "wx/wx.h"
#include "wx/listctrl.h"

class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title);
    void OnFileNew(wxCommandEvent& event);
    void OnFileOpen(wxCommandEvent& event);
    void OnFileClose(wxCommandEvent& event);
    void OnFileExit(wxCommandEvent& event);

    void OnPrevMonth(wxCommandEvent& event);
    void OnNextMonth(wxCommandEvent& event);
    void OnListItemActivated(wxListEvent& event);

    void CreateMenuBar();
    void CreateControls();
    void RefreshControls();
    void RefreshExpenses();

    wxPanel *CreateExpensesNav(wxWindow *parent);
    wxPanel *CreateExpensesView(wxWindow *parent);

private:
    wxDECLARE_EVENT_TABLE();
};

#endif
