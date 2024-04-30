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
