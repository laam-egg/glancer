#ifndef App_INCLUDED
#define App_INCLUDED

#include <wx/wxprec.h>
#include <wx/cmdline.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

class App : public wxApp {
public:
	virtual bool OnInit() override;
	virtual void OnInitCmdLine(wxCmdLineParser& parser) override;
	virtual bool OnCmdLineParsed(wxCmdLineParser& parser) override;

private:
	wxString initialFileName{};
};

// https://docs.wxwidgets.org/trunk/classwx_cmd_line_parser.html#ac7281e9d5dfb7e41c1bf5f9fb959bbee
static const wxCmdLineEntryDesc g_cmdLineDesc[] = {
	{ wxCMD_LINE_PARAM, nullptr, nullptr, "The path to the file to be opened.", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },

	{ wxCMD_LINE_NONE }
};

#endif // App_INCLUDED
