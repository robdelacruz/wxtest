#include "wx/wx.h"
#include "app.h"
#include "frame.h"

DECLARE_APP(MyApp)
IMPLEMENT_APP(MyApp)

bool MyApp::OnInit() {
    MyFrame *frame = new MyFrame(wxT("MyFrame"));
    frame->Show(true);

    return true;
}

