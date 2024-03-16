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

static float const MAX_ZOOM_FACTOR = 5.0f; // 500%
class ImageZoomer {
private:
	bool m_autoSizing;

	int const m_imageWidth, m_imageHeight;
	double const m_imageWidthPerHeightRatio;
	double m_imageSizePerWindowSizeRatio;
	wxSize m_minWindowSize;
	wxSize m_minImageSize;
	wxSize m_desiredWindowSize;
	wxSize m_currentSize;

	wxPoint m_currentPosition;
	wxPoint m_currentPositionDelta;

public:
	ImageZoomer(int imageWidth, int imageHeight)
		: m_autoSizing{ true },
		m_imageWidth{ imageWidth },
		m_imageHeight{ imageHeight },
		m_imageWidthPerHeightRatio{ 1.0 * m_imageWidth / m_imageHeight },
		m_currentPositionDelta{ 0, 0 }
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

		m_currentPosition = { 0, 0 };
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

	wxPoint getCurrentPosition() const {
		return m_currentPosition;
	}

	void adapt(wxSize const& newWindowSize) {
		if (m_autoSizing) {
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
		m_currentPosition = {
			wxCoord{ (newWindowSize.GetWidth() - m_currentSize.GetWidth()) / 2 + m_currentPositionDelta.x },
			wxCoord{ (newWindowSize.GetHeight() - m_currentSize.GetHeight()) / 2 + m_currentPositionDelta.y },
		};
	}

	void zoomBy(int numZoomSteps) {
		m_autoSizing = false;

		double const zoomPercentagePerStep = 0.05;
		double factor = 1.0;
		if (numZoomSteps > 0) {
			factor = 1 + numZoomSteps * zoomPercentagePerStep;
		}
		else if (numZoomSteps < 0) {
			factor = 1 + numZoomSteps * zoomPercentagePerStep;
		}

		int newWidth = wxMax(m_currentSize.GetWidth() * factor, m_minImageSize.GetWidth());
		int newHeight = wxMax(m_currentSize.GetHeight() * factor, m_minImageSize.GetHeight());

		float currentZoomFactor = 1.0f * newHeight / m_imageHeight;
		if (currentZoomFactor > MAX_ZOOM_FACTOR) {
			currentZoomFactor = MAX_ZOOM_FACTOR;
			newWidth = m_imageWidth * currentZoomFactor;
			newHeight = m_imageHeight * currentZoomFactor;
		}

		m_currentSize = { newWidth, newHeight };
	}

	void move(wxPoint const& mouseMoveDelta, wxSize const& windowSize) {
		m_currentPositionDelta += mouseMoveDelta;
		adapt(windowSize);
	}

	int getCurrentZoomFactorByPercentage() const {
		return int(1.0f * m_currentSize.GetHeight() / m_imageHeight * 100);
	}
};

wxDEFINE_EVENT(MOUSE_DRAG_BEGIN_EVENT, wxCommandEvent);
wxDEFINE_EVENT(MOUSE_DRAG_END_EVENT, wxCommandEvent);

class MouseDragEvent;
wxDECLARE_EVENT(MOUSE_DRAG_EVENT, MouseDragEvent);
class MouseDragEvent : public wxCommandEvent {
private:
	wxPoint m_delta;
public:
	MouseDragEvent(wxPoint delta) : wxCommandEvent(MOUSE_DRAG_EVENT), m_delta{ delta } {
	}

	MouseDragEvent(MouseDragEvent const& other) : wxCommandEvent(other) {
		this->SetDelta(other.GetDelta());
	}

	virtual wxEvent* Clone() const override {
		return new MouseDragEvent(*this);
	}

	void SetDelta(wxPoint const& newDelta) {
		m_delta = newDelta;
	}

	wxPoint const& GetDelta() const {
		return m_delta;
	}
};
wxDEFINE_EVENT(MOUSE_DRAG_EVENT, MouseDragEvent);

class MouseDragDetector {
private:
	wxPoint m_lastPos;
	bool m_dragging;
	wxWindow* const m_parent;

public:
	MouseDragDetector(wxWindow* parent)
		: m_parent{ parent },
		m_lastPos{ 0, 0 },
		m_dragging{ false }
	{}

	void handleMouseMove(wxMouseEvent& event) {
		if (event.Dragging() && event.LeftIsDown()) {
			if (m_dragging) {
				wxPoint delta = event.GetPosition() - m_lastPos;
				m_lastPos = event.GetPosition();
				MouseDragEvent e(delta);
				wxPostEvent(m_parent, e);
			}
			else {
				m_dragging = true;
				m_lastPos = event.GetPosition();
				wxCommandEvent e(MOUSE_DRAG_BEGIN_EVENT);
				wxPostEvent(m_parent, e);
			}
		}
		else {
			undrag();
		}
	}

	void handleMouseLeftUp(wxMouseEvent& event) {
		undrag();
	}

private:
	void undrag() {
		if (m_dragging) {
			m_dragging = false;
			wxCommandEvent e(MOUSE_DRAG_END_EVENT);
			wxPostEvent(m_parent, e);
		}
	}
};

wxDEFINE_EVENT(ZOOM_FACTOR_CHANGED, wxCommandEvent);

class RealImagePanel final : public BasePanel {
private:
	wxMemoryDC m_imageDC;
	ImageZoomer m_iz;
	MouseDragDetector m_mouseDragDetector;

public:
	RealImagePanel(wxWindow* parent, wxImage const& image)
		: BasePanel(parent),
		m_imageDC(wxBitmap(image)),
		m_iz(image.GetWidth(), image.GetHeight()),
		m_mouseDragDetector(this)
	{
		Bind(wxEVT_PAINT, &RealImagePanel::onPaintEvent, this);
		Bind(wxEVT_SIZE, &RealImagePanel::onResizeEvent, this);
		Bind(wxEVT_MOUSEWHEEL, &RealImagePanel::onMouseWheel, this);
		Bind(wxEVT_MOTION, &RealImagePanel::onMouseMove, this);
		Bind(wxEVT_LEFT_UP, &RealImagePanel::onMouseLeftUp, this);
		
		Bind(MOUSE_DRAG_EVENT, &RealImagePanel::onMouseDrag, this);
		Bind(MOUSE_DRAG_BEGIN_EVENT, &RealImagePanel::onMouseDragBegin, this);
		Bind(MOUSE_DRAG_END_EVENT, &RealImagePanel::onMouseDragEnd, this);

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
		wxCoord newImageWidth{ m_iz.getCurrentSize().GetWidth() };
		wxCoord newImageHeight{ m_iz.getCurrentSize().GetHeight() };

		wxCommandEvent e(ZOOM_FACTOR_CHANGED);
		e.SetInt(m_iz.getCurrentZoomFactorByPercentage());
		wxPostEvent(this, e);

		dc.StretchBlit(
			m_iz.getCurrentPosition().x,
			m_iz.getCurrentPosition().y,
			newImageWidth,
			newImageHeight,
			&m_imageDC,
			wxCoord{ 0 },
			wxCoord{ 0 },
			wxCoord{ m_imageDC.GetSize().GetWidth() },
			wxCoord{ m_imageDC.GetSize().GetHeight() }
		);
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

	void onMouseMove(wxMouseEvent& event) {
		m_mouseDragDetector.handleMouseMove(event);
	}

	void onMouseLeftUp(wxMouseEvent& event) {
		m_mouseDragDetector.handleMouseLeftUp(event);
	}

	void onMouseDrag(MouseDragEvent& event) {
		m_iz.move(event.GetDelta(), GetSize());
		Refresh(false);
	}

	void onMouseDragBegin(wxCommandEvent& event) {
		SetCursor(wxCURSOR_HAND);
	}

	void onMouseDragEnd(wxCommandEvent& event) {
		SetCursor(wxCURSOR_ARROW);
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

ImagePage::ImagePage(wxWindow* parent, wxString imageFilePath)
	: BasePanel(parent)
{
	wxFileName p(imageFilePath);
	p.Normalize();
	m_imageFilePath = p.GetAbsolutePath();

	Bind(ZOOM_FACTOR_CHANGED, &ImagePage::onZoomFactorChanged, this);

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

void ImagePage::onZoomFactorChanged(wxCommandEvent& event) {
	wxCommandEvent e(MainFrame_CHANGE_TITLE);
	e.SetString(m_imageFilePath + wxString::Format(" (%d%%)", event.GetInt()));
	wxPostEvent(this, e);
}
