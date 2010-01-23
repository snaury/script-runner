#include "script-runner.h"

static const TCHAR* extensions[] = {
    _T(".pl"),
    _T(""),
    NULL
};

static const TCHAR* runners[] = {
    _T("perl.exe"),
    NULL
};

const script_config config = { extensions, runners };
