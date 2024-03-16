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

class ImageZoomer {
private:
	bool m_auto;
	int const m_imageWidth, m_imageHeight;
	double const m_imageWidthPerHeightRatio;
	double m_imageSizePerWindowSizeRatio;
	wxSize m_minWindowSize;
	wxSize m_minImageSize;
	wxSize m_desiredWindowSize;
	wxSize m_currentSize;

public:
	ImageZoomer(int imageWidth, int imageHeight, bool automatic = true)
		: m_auto{ automatic },
		m_imageWidth{ imageWidth },
		m_imageHeight{ imageHeight },
		m_imageWidthPerHeightRatio{ 1.0 * m_imageWidth / m_imageHeight }
	{
		int const minWidth = 200; // the window should not be too small !
		int const minHeight = minWidth / m_imageWidthPerHeightRatio;

		int desiredWidth = wxMax(
			wxMin(m_imageWidth, int(wxDisplay().GetGeometry().GetWidth() * 0.7)),
			minWidth
		);
		int desiredHeight = wxMax(
			wxMin(m_imageHeight, int(wxDisplay().GetGeometry().GetHeight()) * 0.8),
			minHeight
		);

		double ratioDelta = 1.0 * desiredWidth / desiredHeight - m_imageWidthPerHeightRatio;
		if (ratioDelta > 0) {
			desiredHeight = desiredWidth / m_imageWidthPerHeightRatio;
		}
		else {
			desiredWidth = desiredHeight * m_imageWidthPerHeightRatio;
		}

		m_imageSizePerWindowSizeRatio = 1.0 * m_imageWidth / desiredWidth;
		m_minWindowSize = wxSize(minWidth, minHeight);
		m_minImageSize = wxSize(minWidth * m_imageSizePerWindowSizeRatio, minHeight * m_imageSizePerWindowSizeRatio);
		m_desiredWindowSize = wxSize(desiredWidth, desiredHeight);
		m_currentSize = m_desiredWindowSize;
	}

	wxSize getMinWindowSize() const {
		return m_minWindowSize;
	}

	wxSize getWindowDesiredSize() const {
		return m_desiredWindowSize;
	}

	wxSize getCurrentSize() const {
		return m_currentSize;
	}

	void adapt(wxSize const& newWindowSize) {
		if (!m_auto) return;

		int newImageWidth = m_imageSizePerWindowSizeRatio * newWindowSize.GetWidth();
		int newImageHeight = newImageWidth / m_imageWidthPerHeightRatio;
		if (newImageHeight > newWindowSize.GetHeight()) {
			newImageHeight = newWindowSize.GetHeight();
			newImageWidth = newImageHeight * m_imageWidthPerHeightRatio;
		}
		if (newImageWidth > newWindowSize.GetWidth()) {
			newImageWidth = newWindowSize.GetWidth();
			newImageHeight = newImageWidth / m_imageWidthPerHeightRatio;
		}

		m_currentSize = { newImageWidth, newImageHeight };
	}

	void zoomBy(int numZoomSteps) {
		double const zoomPercentagePerStep = 0.05;
		m_auto = false;
		double factor = 1.0;
		if (numZoomSteps > 0) {
			factor = 1 + numZoomSteps * zoomPercentagePerStep;
		}
		else if (numZoomSteps < 0) {
			factor = 1 + numZoomSteps * zoomPercentagePerStep;
		}

		int newWidth = wxMax(m_currentSize.GetWidth() * factor, m_minImageSize.GetWidth());
		int newHeight = wxMax(m_currentSize.GetHeight() * factor, m_minImageSize.GetHeight());
		m_currentSize = { newWidth, newHeight };
	}
};

class RealImagePanel final : public BasePanel {
private:
	wxImage m_image;
	ImageZoomer m_iz;
	double m_accumulatedMouseWheelRotation;

public:
	RealImagePanel(wxWindow* parent, wxImage const& image)
		: BasePanel(parent),
		m_image{ image },
		m_iz(image.GetWidth(), image.GetHeight()),
		m_accumulatedMouseWheelRotation{ 0.0 }
	{
		Bind(wxEVT_PAINT, &RealImagePanel::onPaintEvent, this);
		Bind(wxEVT_SIZE, &RealImagePanel::onResizeEvent, this);
		Bind(wxEVT_MOUSEWHEEL, &RealImagePanel::onMouseWheel, this);

		SetMinSize(m_iz.getMinWindowSize());
	}

	virtual void adjustDesiredSize() override {
		ClientAreaResizeEvent e(m_iz.getWindowDesiredSize());
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

		m_iz.adapt(GetSize());
		int newImageWidth = m_iz.getCurrentSize().GetWidth();
		int newImageHeight = m_iz.getCurrentSize().GetHeight();

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

	void onMouseWheel(wxMouseEvent& event) {
		if (event.ControlDown()) {
			int numZoomSteps = abs(event.GetWheelRotation()) / event.GetWheelRotation();
			m_iz.zoomBy(numZoomSteps);
			Refresh(false);
			return;
		}
		event.Skip();
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
