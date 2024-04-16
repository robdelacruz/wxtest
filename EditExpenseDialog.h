#ifndef EDITEXPENSEDIALOG_H
#define EDITEXPENSEDIALOG_H

#include <vector>
#include "wx/wx.h"
#include "wx/datectrl.h"
#include "db.h"

class EditExpenseDialog : public wxDialog {
public:
    wxString m_desc;
    double m_amt;
    int m_icatsel;
    wxDateTime m_date;
    exp_t *m_xp;
    wxDatePickerCtrl *m_dpDate;

    EditExpenseDialog(wxWindow *parent, exp_t *xp);

private:
    void CreateControls();
    bool TransferDataFromWindow();
    uint64_t GetCatIdFromSelectedIndex(int icatsel);

    wxDECLARE_EVENT_TABLE();
};

#endif

