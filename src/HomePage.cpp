#include "HomePage.h"
#include "Utils.h"

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

		wxButton* aboutButton = new wxButton(this, wxID_ANY, "About...");
		{
			wxImage aboutIcon;
			wxString path = Utils::getMainDir() + "/images/about.png";
			aboutIcon.LoadFile(path);
			aboutButton->SetBitmap(wxBitmap(aboutIcon));
		}

		{
			{
				mainSizer->Add(openLocalImageButton, 1, wxALIGN_LEFT | wxEXPAND);
				mainSizer->Add(takeScreenshotButton, 1, wxALIGN_LEFT | wxEXPAND);
				mainSizer->Add(aboutButton, 1, wxALIGN_LEFT | wxEXPAND);
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
