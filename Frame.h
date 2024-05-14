#ifndef FRAME_H
#define FRAME_H

#include "wx/wx.h"
#include "wx/listctrl.h"
#include "wx/notebook.h"
#include "wx/propgrid/propgrid.h"
#include "db.h"

class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title);
    ~MyFrame();

    void CreateMenuBar();
    void CreateControls();
    wxWindow *CreateNav(wxWindow *parent);
    wxWindow *CreateContentView(wxWindow *parent);
    wxWindow *CreateExpensesView(wxWindow *parent);
    wxWindow *CreateExpensesList(wxWindow *parent);
    wxWindow *CreateExpensePG(wxWindow *parent);
    wxWindow *CreateCategorySummary(wxWindow *parent);
    void RefreshFrame();

    void RefreshMenu();
    void RefreshNav();

    void RefreshExpensesList(uint64_t sel_expid, long sel_row);
    void RefreshSingleExpenseInList(exp_t *xp);
    void RefreshExpensePG(exp_t *xp);
    void RefreshCategorySummary();

    void EditExpense(exp_t *xp);

    void OnFileNew(wxCommandEvent& event);
    void OnFileOpen(wxCommandEvent& event);
    void OnFileClose(wxCommandEvent& event);
    void OnFileExit(wxCommandEvent& event);

    void OnExpenseNew(wxCommandEvent& event);
    void OnExpenseEdit(wxCommandEvent& event);
    void OnExpenseDel(wxCommandEvent& event);
    void OnExpenseCategories(wxCommandEvent& event);

    void OnExpensesPrevious(wxCommandEvent& event);
    void OnExpensesNext(wxCommandEvent& event);
    void OnExpenseListItemSelected(wxListEvent& event);
    void OnExpenseListItemDeselected(wxListEvent& event);
    void OnExpenseListItemActivated(wxListEvent& event);
    void OnExpenseListItemRightClick(wxListEvent& event);
    void OnExpenseListColClick(wxListEvent& event);
    void OnExpensePropertyGridChanged(wxPropertyGridEvent& event);

    void OnNavPageChanged(wxBookCtrlEvent& event);
    void OnNavYearChanged(wxCommandEvent& event);
    void OnNavPrevYear(wxCommandEvent& event);
    void OnNavNextYear(wxCommandEvent& event);
    void OnNavMonthSelected(wxListEvent& event);
    void OnNavYearSelected(wxListEvent& event);

    void OnCategoriesSummaryListColClick(wxListEvent& event);

    void OnNavMonthsListSize(wxSizeEvent& event);

private:
    exp_t *m_propgrid_xp;
    bool m_sortDate = SORT_DOWN;
    bool m_sortDesc = SORT_DOWN;
    bool m_sortAmt = SORT_DOWN;
    bool m_sortCat = SORT_DOWN;
    bool m_sortCattotalName = SORT_DOWN;
    bool m_sortCattotalTotal = SORT_DOWN;

    wxDECLARE_EVENT_TABLE();

};

class NavListView : public wxListView {
public:
    NavListView(wxWindow *parent, wxWindowID winid=wxID_ANY, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=wxLC_REPORT, const wxValidator &validator=wxDefaultValidator, const wxString &name=wxListCtrlNameStr);
    ~NavListView();

    void OnSize(wxSizeEvent& event);

private:
    wxDECLARE_EVENT_TABLE();
};

#endif
