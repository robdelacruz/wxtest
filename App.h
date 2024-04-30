#ifndef APP_H
#define APP_H

#include "wx/wx.h"
#include "expense.h"

enum {
    ID_EXPENSE_NEW = wxID_HIGHEST,
    ID_EXPENSE_EDIT,
    ID_EXPENSE_DEL,
    ID_EXPENSE_CATEGORIES,

    ID_VIEW_MONTHLY,
    ID_VIEW_DAILY,

    ID_NO_OPEN_FILE,
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

#endif
