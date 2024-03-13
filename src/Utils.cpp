#include "Utils.h"
#include <wx/filename.h>
#include <wx/stdpaths.h>

wxString Utils::getMainDir() {
	return wxFileName(
		wxStandardPaths::Get().GetExecutablePath()
	).GetPath();
}
