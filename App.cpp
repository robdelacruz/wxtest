#include "clib.h"
#include "App.h"
#include "Frame.h"

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
//    if (!wxApp::OnInit())
//        return false;

    m_expctx = ctx_new();
    if (ctx_init_from_args(m_expctx, wxGetApp().argc, wxGetApp().argv) != 0) {
        exit(1);
    }

    MyFrame *frame = new MyFrame(wxT("MyFrame"));
    frame->Show(true);
    return true;
}

ExpenseContext *getContext() {
    return wxGetApp().m_expctx;
}

