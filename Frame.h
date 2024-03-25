#ifndef FRAME_H
#define FRAME_H

#include "wx/wx.h"
#include "wx/listctrl.h"

class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title);
    void OnQuit(wxCommandEvent& event);
    void OnListItemActivated(wxListEvent& event);

private:
    DECLARE_EVENT_TABLE()
};

#endif
