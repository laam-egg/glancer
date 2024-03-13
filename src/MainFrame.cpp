#include "MainFrame.h"
#include "HomePage.h"
#include "ImagePage.h"
#include "BasePanel.h"

wxDEFINE_EVENT(MainFrame_CHANGE_TITLE, wxCommandEvent);
wxDEFINE_EVENT(MainFrame_RESIZE_CLIENT_AREA, ClientAreaResizeEvent);

MainFrame::MainFrame(wxString initialFileName) : BaseFrame(NULL, wxID_ANY, APP_VERSION_STRING) {
	Hide();

	Bind(MainFrame_CHANGE_TITLE, &MainFrame::changeTitle, this);
	Bind(MainFrame_RESIZE_CLIENT_AREA, &MainFrame::resizeClientArea, this);

	wxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	BasePanel* homePage = (
		!initialFileName
		? dynamic_cast<BasePanel*>(new HomePage(this))
		: dynamic_cast<BasePanel*>(new ImagePage(this, initialFileName))
	);

	{
		mainSizer->Add(homePage, 1, wxEXPAND | wxALL);
		SetSizerAndFit(mainSizer);
	}

	homePage->adjustDesiredSize();

	Show();
}

void MainFrame::changeTitle(wxCommandEvent& event) {
	wxString const& newTitle = event.GetString();
	if (!newTitle) {
		SetTitle(APP_VERSION_STRING);
	}
	else {
		SetTitle(newTitle + " -- " + APP_VERSION_STRING);
	}
}

void MainFrame::resizeClientArea(ClientAreaResizeEvent& event) {
	SetClientSize(event.GetSize());
	Center();
}
