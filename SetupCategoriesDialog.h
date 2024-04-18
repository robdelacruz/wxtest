#ifndef SETUPCATEGORIESDIALOG_H
#define SETUPCATEGORIESDIALOG_H

#include "wx/wx.h"

class SetupCategoriesDialog : public wxDialog {
public:
    SetupCategoriesDialog(wxWindow *parent);

private:
    void CreateControls();

    wxDECLARE_EVENT_TABLE();
};

#endif

