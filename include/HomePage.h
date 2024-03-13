#ifndef HomePage_INCLUDED
#define HomePage_INCLUDED

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "BasePanel.h"

class HomePage : public BasePanel {
public:
	HomePage(wxWindow* parent);

	virtual void adjustDesiredSize() override;
};

#endif // HomePage_INCLUDED
