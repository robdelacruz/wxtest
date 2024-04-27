#ifndef FRAME_H
#define FRAME_H

#include "wx/wx.h"
#include "wx/listctrl.h"
#include "wx/propgrid/propgrid.h"
#include "db.h"

class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title);
    ~MyFrame();

    void CreateMenuBar();
    void CreateControls();
    wxWindow *CreateNav(wxWindow *parent);
    wxWindow *CreateExpensesView(wxWindow *parent);
    wxWindow *CreateExpensesList(wxWindow *parent);
    wxWindow *CreateExpensePropGrid(wxWindow *parent);
    void RefreshFrame();

    void RefreshMenu();
    void RefreshNav();
    void RefreshExpenses(uint64_t sel_expid, long sel_row);
    void RefreshExpensesList(uint64_t sel_expid, long sel_row);
    void RefreshSingleExpenseInList(exp_t *xp);
    void RefreshExpenseGrid(exp_t *xp);

    void EditExpense(exp_t *xp);

    void OnFileNew(wxCommandEvent& event);
    void OnFileOpen(wxCommandEvent& event);
    void OnFileClose(wxCommandEvent& event);
    void OnFileExit(wxCommandEvent& event);

    void OnExpenseNew(wxCommandEvent& event);
    void OnExpenseEdit(wxCommandEvent& event);
    void OnExpenseDel(wxCommandEvent& event);
    void OnExpenseCategories(wxCommandEvent& event);

    void OnPrevious(wxCommandEvent& event);
    void OnNext(wxCommandEvent& event);
    void OnListItemSelected(wxListEvent& event);
    void OnListItemDeselected(wxListEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnPropertyGridChanged(wxPropertyGridEvent& event);

    void OnNavYearChanged(wxCommandEvent& event);
    void OnNavPrevYear(wxCommandEvent& event);
    void OnNavNextYear(wxCommandEvent& event);
    void OnNavListSelected(wxListEvent& event);

private:
    exp_t *m_propgrid_xp;

    wxDECLARE_EVENT_TABLE();

};

#endif
