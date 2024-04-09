#ifndef FRAME_H
#define FRAME_H

#include "wx/wx.h"
#include "wx/listctrl.h"
#include "wx/propgrid/propgrid.h"

#include "db.h"

class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title);
    void OnFileNew(wxCommandEvent& event);
    void OnFileOpen(wxCommandEvent& event);
    void OnFileClose(wxCommandEvent& event);
    void OnFileExit(wxCommandEvent& event);

    void OnNavYear(wxSpinEvent& event);
    void OnNavMonth(wxCommandEvent& event);
    void OnPrevMonth(wxCommandEvent& event);
    void OnNextMonth(wxCommandEvent& event);
    void OnListItemSelected(wxListEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnPropertyGridChanged(wxPropertyGridEvent& event);

    void CreateMenuBar();
    void CreateControls();
    void ShowControls();
    void RefreshNav();
    void RefreshExpenses();

    wxWindow *CreateExpensesNav(wxWindow *parent);
    wxWindow *CreateExpensesView(wxWindow *parent);
    wxWindow *CreateExpensesList(wxWindow *parent);
    wxWindow *CreateExpensePropGrid(wxWindow *parent);

    void ClearExpenseList();
    void RefreshExpenseList();
    void ClearExpenseGrid();
    void RefreshExpenseGrid();

private:
    wxDECLARE_EVENT_TABLE();
};

#endif
