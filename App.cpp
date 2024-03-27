#include "App.h"
#include "Frame.h"

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit() {
    MyFrame *frame;

    m_expctx = ctx_init_args(wxGetApp().argc, wxGetApp().argv);
    if (m_expctx == NULL)
        exit(1);

    frame = new MyFrame(wxT("MyFrame"));
    frame->Show(true);

    return true;
}

ExpenseContext *getContext() {
    return wxGetApp().m_expctx;
}

