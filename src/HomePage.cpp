#include "HomePage.h"
#include "Utils.h"
#include <wx/hyperlink.h>

class HomePageAboutPanel : public wxPanel {
public:
	HomePageAboutPanel(wxWindow* parent) : wxPanel(parent) {
		wxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);

		SetFont(GetFont().Scale(0.7));
		auto* appCopyrightText = new wxStaticText(this, wxID_ANY, L"Copyright (C) 2024 V\x169 T\xF9ng L\xE2m. ");
		{
			appCopyrightText->SetForegroundColour(wxColor(70, 70, 70));
		}

		auto* aboutLink = new wxHyperlinkCtrl(this, wxID_ANY, "More information", "https://github.com/laam-egg/glancer");
		{
			aboutLink->SetForegroundColour(wxColor(70, 70, 70));
		}

		{
			{
				mainSizer->Add(appCopyrightText, 0);
				mainSizer->Add(aboutLink, 0);
			}
			SetSizerAndFit(mainSizer);
		}
	}
};

class HomePageButtonPanel : public wxPanel {
public:
	HomePageButtonPanel(wxWindow* parent) : wxPanel(parent) {
		wxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

		wxButton* openLocalImageButton = new wxButton(this, wxID_ANY, "Open a Local Image");
		{
			wxImage openLocalImageIcon;
			wxString path = Utils::getMainDir() + "/images/open-local-image.png";
			openLocalImageIcon.LoadFile(path);
			openLocalImageButton->SetBitmap(wxBitmap(openLocalImageIcon));
		}

		wxButton* takeScreenshotButton = new wxButton(this, wxID_ANY, "Take a Screenshot");
		{
			wxImage takeScreenshotIcon;
			wxString path = Utils::getMainDir() + "/images/take-screenshot.png";
			takeScreenshotIcon.LoadFile(path);
			takeScreenshotButton->SetBitmap(wxBitmap(takeScreenshotIcon));
		}

		{
			{
				mainSizer->Add(openLocalImageButton, 1, wxALIGN_LEFT | wxEXPAND);
				mainSizer->Add(takeScreenshotButton, 1, wxALIGN_LEFT | wxEXPAND);
			}
			SetSizerAndFit(mainSizer);
		}
	}
};

HomePage::HomePage(wxWindow* parent) : BasePanel(parent) {
	wxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	wxStaticText* appNameText = new wxStaticText(this, wxID_ANY, "Glancer");
	{
		appNameText->SetFont(wxFont(
			int(GetFont().GetPointSize() * 2.5),
			wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL
		));
	}

	wxStaticText* appSloganText = new wxStaticText(this, wxID_ANY, "a fast image viewer and editor");
	{
		appSloganText->SetForegroundColour(wxColor(70, 70, 70));
	}

	{
		{
			mainSizer->AddStretchSpacer();
			mainSizer->Add(appNameText, 0, wxALIGN_CENTER);
			mainSizer->Add(appSloganText, 0, wxALIGN_CENTER);
			mainSizer->AddSpacer(15);
			mainSizer->Add(new HomePageButtonPanel(this), 0, wxALIGN_CENTER);
			mainSizer->AddStretchSpacer();
			mainSizer->Add(new HomePageAboutPanel(this), 0, wxALIGN_LEFT);
		}
		SetSizerAndFit(mainSizer);
	}

	wxSize sz = GetSize();
	sz.SetWidth(int(sz.GetWidth() * 2.5));
	sz.SetHeight(int(sz.GetWidth() * 0.6));
	SetMinSize(sz);
}

void HomePage::adjustDesiredSize() {
}
