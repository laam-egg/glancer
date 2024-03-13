#include "BaseFrame.h"

BaseFrame::BaseFrame() : wxFrame() {
	RunCommonInitialization();
}

BaseFrame::BaseFrame(wxWindow* parent,
	wxWindowID id,
	const wxString& title,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name
) : wxFrame(parent, id, title, pos, size, style, name) {
	RunCommonInitialization();
}

void BaseFrame::RunCommonInitialization() {
	int pointSize = GetFont().GetPointSize();
	pointSize *= 1.5;
	wxFont defaultFont(pointSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_LIGHT);
	SetFont(defaultFont);
}
