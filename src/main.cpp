#include "main.h"
#include "MainFrame.h"

wxIMPLEMENT_APP(App);

bool App::OnInit() {
	if (!wxApp::OnInit()) {
		return false;
	}

	wxInitAllImageHandlers();

	auto* mainFrame = new MainFrame(initialFileName);
	mainFrame->Show();

	return true;
}

void App::OnInitCmdLine(wxCmdLineParser& parser) {
	wxApp::OnInitCmdLine(parser);
	parser.SetDesc(g_cmdLineDesc);
	parser.SetSwitchChars("-");
}

bool App::OnCmdLineParsed(wxCmdLineParser& parser) {
	if (!wxApp::OnCmdLineParsed(parser)) {
		return false;
	}

	int const numParams = parser.GetParamCount();
	if (numParams >= 1) {
		initialFileName = parser.GetParam(0);
	}

	return true;
}
