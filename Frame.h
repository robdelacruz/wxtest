#ifndef FRAME_H
#define FRAME_H

#include "wx/wx.h"
#include "wx/listctrl.h"
#include "wx/propgrid/propgrid.h"

#include "db.h"

class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title);
    void CreateMenuBar();
    void CreateControls();
    wxWindow *CreateExpensesNav(wxWindow *parent);
    wxWindow *CreateExpensesView(wxWindow *parent);
    wxWindow *CreateExpensesList(wxWindow *parent);
    wxWindow *CreateExpensePropGrid(wxWindow *parent);
    void ShowControls();

    void RefreshNav();
    void RefreshExpenses();
    void ClearExpenseList();
    void RefreshExpenseList();
    void ClearExpenseGrid();
    void RefreshExpenseGrid();

    void EditExpense(exp_t *xp);

    void OnFileNew(wxCommandEvent& event);
    void OnFileOpen(wxCommandEvent& event);
    void OnFileClose(wxCommandEvent& event);
    void OnFileExit(wxCommandEvent& event);

    void OnExpenseNew(wxCommandEvent& event);
    void OnExpenseEdit(wxCommandEvent& event);

    void OnPrevMonth(wxCommandEvent& event);
    void OnNextMonth(wxCommandEvent& event);
    void OnListItemSelected(wxListEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnPropertyGridChanged(wxPropertyGridEvent& event);

    void OnNavYear(wxSpinEvent& event);
    void OnNavMonth(wxListEvent& event);






private:
    wxDECLARE_EVENT_TABLE();
};

#endif
