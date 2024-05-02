#include "clib.h"
#include "App.h"
#include "Frame.h"

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    m_expctx = ctx_new();
    if (ctx_init_from_args(m_expctx, argc, argv) != 0) {
        exit(1);
    }

    MyFrame *frame = new MyFrame(wxT("MyFrame"));
    frame->Show(true);
    return true;
}

ExpenseContext *getContext() {
    return wxGetApp().m_expctx;
}

wxString formatAmount(double amt, const char *fmt) {
    return wxString::Format("%'9.2f", amt);
}
wxString formatDate(date_t dt, const char *fmt) {
    return wxDateTime(dt).Format(fmt);
}
wxString formatDate(int year, int month, int day, const char *fmt) {
    return wxDateTime(date_from_cal(year,month,day)).Format(fmt);
}

// listctrl sorting comparers
int cmpExpID(exp_t *exp1, exp_t *exp2) {
    if (exp1->expid < exp2->expid) return -1;
    if (exp1->expid > exp2->expid) return 1;
    return 0;
}
int wxCALLBACK cmpExpDateAsc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData) {
    exp_t *exp1 = (exp_t *) item1;
    exp_t *exp2 = (exp_t *) item2;

    // Compare by date ascending, then expids.
    if (exp1->date < exp2->date) return -1;
    if (exp1->date > exp2->date) return 1;
    return cmpExpID(exp1, exp2);
}
int wxCALLBACK cmpExpDateDesc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData) {
    exp_t *exp1 = (exp_t *) item1;
    exp_t *exp2 = (exp_t *) item2;

    // Compare by date descending, then expids.
    if (exp1->date < exp2->date) return 1;
    if (exp1->date > exp2->date) return -1;
    return cmpExpID(exp1, exp2);
}
int wxCALLBACK cmpExpDescriptionAsc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData) {
    exp_t *exp1 = (exp_t *) item1;
    exp_t *exp2 = (exp_t *) item2;
    int cmp = wxString::FromUTF8(exp1->desc->s).CmpNoCase(wxString::FromUTF8(exp2->desc->s));
    if (cmp != 0) return cmp;
    return cmpExpID(exp1, exp2);
}
int wxCALLBACK cmpExpDescriptionDesc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData) {
    exp_t *exp1 = (exp_t *) item1;
    exp_t *exp2 = (exp_t *) item2;
    int cmp = wxString::FromUTF8(exp1->desc->s).CmpNoCase(wxString::FromUTF8(exp2->desc->s));
    if (cmp != 0) return -cmp;
    return cmpExpID(exp1, exp2);
}
int wxCALLBACK cmpExpAmtAsc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData) {
    exp_t *exp1 = (exp_t *) item1;
    exp_t *exp2 = (exp_t *) item2;

    if (exp1->amt < exp2->amt) return -1;
    if (exp1->amt > exp2->amt) return 1;
    return cmpExpID(exp1, exp2);
}
int wxCALLBACK cmpExpAmtDesc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData) {
    exp_t *exp1 = (exp_t *) item1;
    exp_t *exp2 = (exp_t *) item2;

    if (exp1->amt < exp2->amt) return 1;
    if (exp1->amt > exp2->amt) return -1;
    return cmpExpID(exp1, exp2);
}
int wxCALLBACK cmpExpCatAsc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData) {
    exp_t *exp1 = (exp_t *) item1;
    exp_t *exp2 = (exp_t *) item2;

    int cmp = wxString::FromUTF8(exp1->catname->s).CmpNoCase(wxString::FromUTF8(exp2->catname->s));
    if (cmp != 0) return cmp;
    return cmpExpID(exp1, exp2);
}
int wxCALLBACK cmpExpCatDesc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData) {
    exp_t *exp1 = (exp_t *) item1;
    exp_t *exp2 = (exp_t *) item2;

    int cmp = wxString::FromUTF8(exp1->catname->s).CmpNoCase(wxString::FromUTF8(exp2->catname->s));
    if (cmp != 0) return -cmp;
    return cmpExpID(exp1, exp2);
}
