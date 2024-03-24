#ifndef APP_H
#define APP_H

#include "wx/wx.h"

class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

DECLARE_APP(MyApp);

#endif
