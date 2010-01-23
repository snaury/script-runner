#include "script-runner.h"

static const TCHAR* extensions[] = {
    _T(".tcl"),
    _T(".kit"),
    _T(""),
    NULL
};

static const TCHAR* runners[] = {
    _T("tclsh.exe"),
    _T("tclkitsh.exe"),
    NULL
};

const script_config config = { extensions, runners };
