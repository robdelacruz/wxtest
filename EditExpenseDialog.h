#ifndef EDITEXPENSEDIALOG_H
#define EDITEXPENSEDIALOG_H

#include "wx/wx.h"

class EditExpenseDialog : public wxDialog {
public:
    EditExpenseDialog(wxWindow *parent);

private:
    wxDECLARE_EVENT_TABLE();
};

#endif

