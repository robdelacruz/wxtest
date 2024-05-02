#ifndef APP_H
#define APP_H

#include "wx/wx.h"
#include "expense.h"

enum {
    ID_EXPENSE_NEW = wxID_HIGHEST,
    ID_EXPENSE_EDIT,
    ID_EXPENSE_DEL,
    ID_EXPENSE_CATEGORIES,

    ID_FRAME_CAPTION,
    ID_MAIN_PANEL,

    ID_CONTENT_NB,

    ID_NAV_NB,
    ID_NAV_YEAR,
    ID_NAV_PREVYEAR,
    ID_NAV_NEXTYEAR,
    ID_NAV_YEARSLIST,
    ID_NAV_MONTHSLIST,

    ID_EXPENSES_PANEL,
    ID_EXPENSES_PREV,
    ID_EXPENSES_HEADING,
    ID_EXPENSES_NEXT,
    ID_EXPENSES_FILTERTEXT,
    ID_EXPENSES_LIST,
    ID_EXPENSE_GRID,

    ID_CATEGORIES_SUMMARY_PANEL,
    ID_CATEGORIES_SUMMARY_LIST,

    ID_EDITEXPENSE_DESCRIPTION,
    ID_EDITEXPENSE_AMOUNT,
    ID_EDITEXPENSE_CATEGORY,
    ID_EDITEXPENSE_DATE,

    ID_SETUPCATEGORIES_HEADING,
    ID_SETUPCATEGORIES_LIST,

    ID_COUNT
};

#define BTN_HEIGHT 25

class MyApp : public wxApp {
public:
    ExpenseContext *m_expctx;

    virtual bool OnInit();
};

wxDECLARE_APP(MyApp);

ExpenseContext *getContext();
wxString formatAmount(double amt, const char *fmt="%'9.2f");
wxString formatDate(date_t dt, const char *fmt);
wxString formatDate(int year, int month, int day, const char *fmt);

int cmpExpID(exp_t *exp1, exp_t *exp2);
int wxCALLBACK cmpExpDateAsc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData);
int wxCALLBACK cmpExpDateDesc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData);
int wxCALLBACK cmpExpDescriptionAsc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData);
int wxCALLBACK cmpExpDescriptionDesc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData);
int wxCALLBACK cmpExpAmtAsc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData);
int wxCALLBACK cmpExpAmtDesc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData);
int wxCALLBACK cmpExpCatAsc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData);
int wxCALLBACK cmpExpCatDesc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData);
int wxCALLBACK cmpCatSummaryNameAsc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData);
int wxCALLBACK cmpCatSummaryNameDesc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData);
int wxCALLBACK cmpCatSummarySubtotalAsc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData);
int wxCALLBACK cmpCatSummarySubtotalDesc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData);

#endif
