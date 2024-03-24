#ifndef FRAME_H
#define FRAME_H

extern "C" {
#include "clib.h"
#include "db.h"
}
#include "wx/wx.h"
#include "wx/listctrl.h"
#include "clib.h"
#include "db.h"

class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title, sqlite3 *db=NULL, char *dbfile=NULL);
    void OnQuit(wxCommandEvent& event);
    void OnListItemActivated(wxListEvent& event);

private:
    DECLARE_EVENT_TABLE()

    sqlite3 *m_db = NULL;
    str_t *m_dbfile;
};

#endif
