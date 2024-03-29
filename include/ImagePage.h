#ifndef ImagePage_INCLUDED
#define ImagePage_INCLUDED

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "BasePanel.h"

class ImagePage : public BasePanel {
public:
	ImagePage(wxWindow* parent, wxString imageFilePath);
	virtual void adjustDesiredSize() override;

	void onZoomFactorChanged(wxCommandEvent& event);

private:
	BasePanel* initializeMainPanel(wxString const& imageFileName);
	BasePanel* m_currentMainPanel;
	wxString m_imageFilePath;
};

#endif // ImagePage_INCLUDED
