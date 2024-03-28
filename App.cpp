#include "clib.h"
#include "App.h"
#include "Frame.h"

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit() {
    MyFrame *frame;
    str_t *err = str_new(0);

    m_expctx = ctx_new();
    if (ctx_init_from_args(m_expctx, wxGetApp().argc, wxGetApp().argv, err) != 0) {
        printf("%s\n", err->s);
        exit(1);
    }

    frame = new MyFrame(wxT("MyFrame"));
    frame->Show(true);
    return true;
}

ExpenseContext *getContext() {
    return wxGetApp().m_expctx;
}

