#include "script-runner.h"

static const TCHAR* extensions[] = {
    _T(".py"),
    _T(""),
    NULL
};

static const TCHAR* runners[] = {
    _T("python.exe"),
    NULL
};

const script_config config = { extensions, runners };

