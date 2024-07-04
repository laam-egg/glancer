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

wxDEFINE_EVENT(ZOOM_FACTOR_CHANGED, wxCommandEvent);

constexpr float const MAX_ZOOM_FACTOR = 25.0f; // 500%
constexpr float const MIN_ZOOM_FACTOR = 0.02f; // 2%

class ImageZoomer {
private:
	wxWindow* const m_windowAgent;

	bool m_autoSizing;

	int const m_imageWidth, m_imageHeight;
	double const m_imageWidthPerHeightRatio;
	double m_imageSizePerWindowSizeRatio;
	float m_currentZoomFactor;
	wxSize m_minWindowSize;
	wxSize m_minImageSize;
	wxSize m_desiredWindowSize;
	wxSize m_currentSize;

	wxPoint m_currentPosition;
	wxPoint m_currentPositionDelta;

	void onZoomFactorChanged() const {
		if (m_windowAgent != nullptr) {
			wxCommandEvent e(ZOOM_FACTOR_CHANGED);
			e.SetInt(getCurrentZoomFactorByPercentage());
			wxPostEvent(m_windowAgent, e);
		}
	}

public:
	/**
	 * @param windowAgent The wxWindow that will be used for sending events,
	 * especially the `ZOOM_FACTOR_CHANGED` events. This can be null, in
	 * which case the events will never be fired.
	 */
	ImageZoomer(wxWindow* windowAgent, int imageWidth, int imageHeight)
		: m_windowAgent{ windowAgent },
		m_autoSizing{ true },
		m_imageWidth{ imageWidth },
		m_imageHeight{ imageHeight },
		m_imageWidthPerHeightRatio{ 1.0 * m_imageWidth / m_imageHeight },
		m_currentPositionDelta{ 0, 0 }
	{
		int const minClientAreaWidth = 200; // the window should not be too small !
		int const minClientAreaHeight = minClientAreaWidth / m_imageWidthPerHeightRatio;

		int desiredWidth = wxMax(
			wxMin(m_imageWidth, int(wxDisplay().GetGeometry().GetWidth() * 0.7)),
			minClientAreaWidth
		);
		int desiredHeight = wxMax(
			wxMin(m_imageHeight, int(wxDisplay().GetGeometry().GetHeight()) * 0.8),
			minClientAreaHeight
		);

		double ratioDelta = 1.0 * desiredWidth / desiredHeight - m_imageWidthPerHeightRatio;
		if (ratioDelta > 0) {
			desiredHeight = desiredWidth / m_imageWidthPerHeightRatio;
		}
		else {
			desiredWidth = desiredHeight * m_imageWidthPerHeightRatio;
		}

		m_imageSizePerWindowSizeRatio = 1.0 * m_imageWidth / desiredWidth;
		m_minWindowSize = wxSize(minClientAreaWidth, minClientAreaHeight);
		m_minImageSize = wxSize(minClientAreaWidth * m_imageSizePerWindowSizeRatio, minClientAreaHeight * m_imageSizePerWindowSizeRatio);
		m_desiredWindowSize = wxSize(desiredWidth, desiredHeight);
		m_currentSize = m_desiredWindowSize;

		m_currentPosition = { 0, 0 };

		m_currentZoomFactor = 1.0f * m_currentSize.GetWidth() / m_imageWidth;

		onZoomFactorChanged();
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
			m_currentZoomFactor = 1.0f * newImageWidth / m_imageWidth;
			onZoomFactorChanged();
		}
		m_currentPosition = {
			wxCoord{ (newWindowSize.GetWidth() - m_currentSize.GetWidth()) / 2 + m_currentPositionDelta.x },
			wxCoord{ (newWindowSize.GetHeight() - m_currentSize.GetHeight()) / 2 + m_currentPositionDelta.y },
		};
	}

	void zoomBy(int numZoomSteps) {
		m_autoSizing = false;

		constexpr float const zoomFactorIncrementPerStep = 0.01f;
		float zoomFactorDelta = numZoomSteps * zoomFactorIncrementPerStep;
		if (zoomFactorDelta == 0.0f) return;

		m_currentZoomFactor += zoomFactorDelta;

		int newWidth = wxMax(m_imageWidth * m_currentZoomFactor, m_minImageSize.GetWidth());
		int newHeight;

		if (m_currentZoomFactor > MAX_ZOOM_FACTOR) {
			m_currentZoomFactor = MAX_ZOOM_FACTOR;
			newWidth = m_imageWidth * MAX_ZOOM_FACTOR;
			newHeight = m_imageHeight * MAX_ZOOM_FACTOR;
		} else if (m_currentZoomFactor < MIN_ZOOM_FACTOR) {
			m_currentZoomFactor = MIN_ZOOM_FACTOR;
			newWidth = m_imageWidth * MIN_ZOOM_FACTOR;
			newHeight = m_imageHeight * MIN_ZOOM_FACTOR;
		} else {
			if (newWidth == m_minImageSize.GetWidth()) {
				newHeight = m_minImageSize.GetHeight();
			} else {
				newHeight = m_imageHeight * m_currentZoomFactor;
			}
		}

		if (zoomFactorDelta < 0.0f) {
			// Zooming out
			constexpr int const MIN_NUM_PIXELS = 50;
			if ((newWidth < MIN_NUM_PIXELS || newHeight < MIN_NUM_PIXELS) && (newWidth <= m_imageWidth /*which implies newHeight <= m_imageHeight as well*/)) {
				// Image is so small that even MIN_ZOOM_FACTOR
				// cannot serve as a proper limit for zooming-out.
				// Stop zooming out.
				newWidth = m_imageWidth;
				newHeight = m_imageHeight;
				m_currentZoomFactor = 1.0f;
				onZoomFactorChanged();
				return;
			}
		}

		m_currentSize = { newWidth, newHeight };
		onZoomFactorChanged();
	}

	void move(wxPoint const& mouseMoveDelta, wxSize const& windowSize) {
		m_currentPositionDelta += mouseMoveDelta;
		adapt(windowSize);
	}

	inline int getCurrentZoomFactorByPercentage() const {
		return int(m_currentZoomFactor * 100);
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

class RealImagePanel final : public BasePanel {
private:
	wxBitmap m_imageBitmap;
	wxMemoryDC m_imageDC;
	ImageZoomer m_iz;
	MouseDragDetector m_mouseDragDetector;

public:
	RealImagePanel(wxWindow* parent, wxImage const& image)
		: BasePanel(parent),
		m_imageBitmap(image),
		m_imageDC(m_imageBitmap),
		m_iz(this, image.GetWidth(), image.GetHeight()),
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
			int rotation = event.GetWheelRotation();
			int numZoomSteps = rotation == 0 ? 0 : (rotation > 0 ? 1 : -1);
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

ImagePage::ImagePage(wxWindow* parent, wxString input_imageFilePath)
	: BasePanel(parent)
{
	wxFileName p(input_imageFilePath);
	p.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_TILDE | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG | wxPATH_NORM_SHORTCUT);
	m_imageFilePath = p.GetAbsolutePath();

	Bind(ZOOM_FACTOR_CHANGED, &ImagePage::onZoomFactorChanged, this);

	wxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	m_currentMainPanel = initializeMainPanel(m_imageFilePath);

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
	e.SetString(wxString::Format("(%d%%) ", event.GetInt()) + m_imageFilePath);
	wxPostEvent(this, e);
}
