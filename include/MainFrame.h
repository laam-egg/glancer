#ifndef MainFrame_INCLUDED
#define MainFrame_INCLUDED

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "BaseFrame.h"

#define APP_VERSION_STRING "Glancer 1.0"

class ClientAreaResizeEvent;

wxDECLARE_EVENT(MainFrame_CHANGE_TITLE, wxCommandEvent);
wxDECLARE_EVENT(MainFrame_RESIZE_CLIENT_AREA, ClientAreaResizeEvent);

class ClientAreaResizeEvent : public wxCommandEvent {
public:
	ClientAreaResizeEvent(wxSize newSize) : wxCommandEvent(MainFrame_RESIZE_CLIENT_AREA), m_newSize{ newSize } {
	}

	ClientAreaResizeEvent(ClientAreaResizeEvent const& other) : wxCommandEvent(other) {
		this->SetSize(other.GetSize());
	}

	virtual wxEvent* Clone() const override {
		return new ClientAreaResizeEvent(*this);
	}

	void SetSize(wxSize newSize) {
		m_newSize = newSize;
	}

	wxSize GetSize() const {
		return m_newSize;
	}

private:
	wxSize m_newSize;
};

class MainFrame : public BaseFrame {
public:
	/**
	 * @param initialFileName - The path to an image file to be opened initially, or an empty string.
	 */
	MainFrame(wxString initialFileName);

	void changeTitle(wxCommandEvent& event);

	void resizeClientArea(ClientAreaResizeEvent& event);
};

#endif // MainFrame_INCLUDED
