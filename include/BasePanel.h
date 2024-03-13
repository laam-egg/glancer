#ifndef BasePanel_INCLUDED
#define BasePanel_INCLUDED

class BasePanel : public wxPanel {
public:
	using wxPanel::wxPanel;

	virtual void adjustDesiredSize() = 0;
};

#endif // BasePanel_INCLUDED
