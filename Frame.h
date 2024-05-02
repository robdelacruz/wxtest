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

private:
    exp_t *m_propgrid_xp;
    bool m_sortDate = false;
    bool m_sortDesc = false;
    bool m_sortAmt = false;
    bool m_sortCat = false;
    bool m_sortCatSummaryName = false;
    bool m_sortCatSummarySubtotal = false;

    wxDECLARE_EVENT_TABLE();

};

#endif
