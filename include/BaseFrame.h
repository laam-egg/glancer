#ifndef BaseFrame_INCLUDED
#define BaseFrame_INCLUDED

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

class BaseFrame : public wxFrame {
public:
	BaseFrame();
	BaseFrame(wxWindow* parent,
		wxWindowID id,
		const wxString& title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_FRAME_STYLE,
		const wxString& name = wxASCII_STR(wxFrameNameStr));

private:
	void RunCommonInitialization();
};

#endif // BaseFrame_INCLUDED
