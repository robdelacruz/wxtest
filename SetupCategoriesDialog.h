#ifndef SETUPCATEGORIESDIALOG_H
#define SETUPCATEGORIESDIALOG_H

#include "wx/wx.h"

class SetupCategoriesDialog : public wxDialog {
public:
    SetupCategoriesDialog(wxWindow *parent);

private:
    void CreateControls();
    void ShowControls();
    void Refresh(uint64_t sel_catid, int sel_row);
    void OnListBoxSelected(wxCommandEvent& event);
    void OnAdd(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif

