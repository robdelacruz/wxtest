#ifndef APP_H
#define APP_H

#include "wx/wx.h"
#include "expense.h"

class MyApp : public wxApp {
public:
    ExpenseContext *m_expctx;

    virtual bool OnInit();
};

wxDECLARE_APP(MyApp);

ExpenseContext *getContext();

#endif
