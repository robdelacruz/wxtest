#ifndef APP_H
#define APP_H

#include "wx/wx.h"
#include "expense.h"

enum {
    ID_EXPENSE_NEW = wxID_HIGHEST,
    ID_EXPENSE_EDIT,
    ID_EXPENSE_DEL,
    ID_EXPENSE_CATEGORIES,

    ID_NO_OPEN_FILE,
    ID_MAIN_SPLIT,

    ID_NAV_YEAR_SPIN,
    ID_NAV_MONTH_LIST,

    ID_EXPENSES_PANEL,
    ID_EXPENSES_HEADING,
    ID_EXPENSES_LIST,
    ID_EXPENSE_GRID,

    ID_EXPENSES_PREV,
    ID_EXPENSES_NEXT,
    ID_EXPENSES_FILTERTEXT,

    ID_EDITEXPENSE_DESCRIPTION,
    ID_EDITEXPENSE_AMOUNT,
    ID_EDITEXPENSE_CATEGORY,
    ID_EDITEXPENSE_DATE,

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

#endif
