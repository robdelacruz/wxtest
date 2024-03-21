#include "wx/wx.h"

class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title);

    void OnQuit(wxCommandEvent& event);
    void OnListItemActivated(wxListEvent& event);

private:
    DECLARE_EVENT_TABLE()
};

