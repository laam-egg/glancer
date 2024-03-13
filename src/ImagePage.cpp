#include "ImagePage.h"
#include "MainFrame.h"
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/display.h>
#include <wx/dcbuffer.h>

class ErrorImagePanel final : public BasePanel {
public:
	ErrorImagePanel(wxWindow* parent, wxString fileName, wxString errorMessage) : BasePanel(parent) {
		auto* firstLine = new wxStaticText(this, wxID_ANY, "Could not open file:");

		auto* fileNameLine = new wxStaticText(this, wxID_ANY, fileName);

		auto* errorMessageLine = new wxStaticText(this, wxID_ANY, errorMessage);
		{
			errorMessageLine->SetForegroundColour(wxColor(70, 70, 70));
		}

		wxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
		{
			{
				mainSizer->AddStretchSpacer();
				mainSizer->Add(firstLine, 0, wxALIGN_CENTER);
				mainSizer->Add(fileNameLine, 0, wxALIGN_CENTER);
				mainSizer->Add(errorMessageLine, 0, wxALIGN_CENTER);
				mainSizer->AddStretchSpacer();
			}
			SetSizerAndFit(mainSizer);
		}

		wxSize sz = GetSize();
		sz.SetWidth(int(sz.GetWidth() * 1.2));
		sz.SetHeight(int(sz.GetWidth() * 0.6));
		SetMinSize(sz);
	}

	virtual void adjustDesiredSize() override {
	}
};

class RealImagePanel final : public BasePanel {
private:
	wxImage m_image;
	wxSize m_desiredSize;
	double m_imageWidthPerHeightRatio;
	double m_imageSizePerWindowSizeRatio;

public:
	RealImagePanel(wxWindow* parent, wxImage const& image) : BasePanel(parent), m_image{ image } {
		Bind(wxEVT_PAINT, &RealImagePanel::onPaintEvent, this);
		Bind(wxEVT_SIZE, &RealImagePanel::onResizeEvent, this);

		m_imageWidthPerHeightRatio = 1.0 * image.GetWidth() / image.GetHeight();
		int const minWidth = 200; // the window should not be too small !
		int const minHeight = minWidth / m_imageWidthPerHeightRatio;

		int desiredWidth = wxMax(
			wxMin(image.GetWidth(), int(wxDisplay().GetGeometry().GetWidth() * 0.7)),
			minWidth
		);
		int desiredHeight = wxMax(
			wxMin(image.GetHeight(), int(wxDisplay().GetGeometry().GetHeight()) * 0.8),
			minHeight
		);

		double ratioDelta = 1.0 * desiredWidth / desiredHeight - m_imageWidthPerHeightRatio;
		if (ratioDelta > 0) {
			desiredHeight = desiredWidth / m_imageWidthPerHeightRatio;
		}
		else {
			desiredWidth = desiredHeight * m_imageWidthPerHeightRatio;
		}

		m_imageSizePerWindowSizeRatio = 1.0 * image.GetWidth() / desiredWidth;
		SetMinSize(wxSize(minWidth, minHeight));
		m_desiredSize = wxSize(desiredWidth, desiredHeight);
	}

	virtual void adjustDesiredSize() override {
		ClientAreaResizeEvent e(m_desiredSize);
		wxPostEvent(this, e);
	}

	void onPaintEvent(wxPaintEvent& event) {
		wxBufferedPaintDC bufferedDC(this);
		render(bufferedDC);
	}

	void onResizeEvent(wxSizeEvent& event) {
		wxClientDC dc(this);
		wxBufferedDC bufferedDC(&dc);
		render(bufferedDC);
	}

	void render(wxDC& dc) {
		dc.Clear();
		int newImageWidth = m_imageSizePerWindowSizeRatio * GetSize().GetWidth();
		int newImageHeight = newImageWidth / m_imageWidthPerHeightRatio;
		if (newImageHeight > GetSize().GetHeight()) {
			newImageHeight = GetSize().GetHeight();
			newImageWidth = newImageHeight * m_imageWidthPerHeightRatio;
		}
		if (newImageWidth > GetSize().GetWidth()) {
			newImageWidth = GetSize().GetWidth();
			newImageHeight = newImageWidth / m_imageWidthPerHeightRatio;
		}

		wxImage scaledImage = m_image.Scale(
			newImageWidth,
			newImageHeight
		);
		wxBitmap bitmap(scaledImage, dc);

		dc.DrawBitmap(bitmap, wxPoint(
			// Center the image on screen
			(GetSize().GetWidth() - newImageWidth) / 2,
			(GetSize().GetHeight() - newImageHeight) / 2
		));
	}
};

BasePanel* ImagePage::initializeMainPanel(wxString const& imageFilePath) {
	if (!wxFile::Exists(imageFilePath)) {
		return new ErrorImagePanel(this, imageFilePath, "The system cannot find the file specified.");
	}

	wxImage image;
	return (
		image.LoadFile(imageFilePath)
		? dynamic_cast<BasePanel*>(new RealImagePanel(this, image))
		: dynamic_cast<BasePanel*>(new ErrorImagePanel(this, imageFilePath, ""))
	);
}

ImagePage::ImagePage(wxWindow* parent, wxString imageFilePath) : BasePanel(parent) {
	wxFileName p(imageFilePath);
	p.Normalize();
	imageFilePath = p.GetAbsolutePath();
	wxCommandEvent e(MainFrame_CHANGE_TITLE);
	e.SetString(imageFilePath);
	wxPostEvent(this, e);
	wxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	m_currentMainPanel = initializeMainPanel(imageFilePath);

	{
		{
			mainSizer->Add(m_currentMainPanel, 1, wxEXPAND);
		}
		SetSizer(mainSizer);
	}
}

void ImagePage::adjustDesiredSize() {
	m_currentMainPanel->adjustDesiredSize();
}
