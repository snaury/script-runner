#include "script-runner.h"

static const TCHAR* extensions[] = {
    _T(".rb"),
    _T(""),
    NULL
};

static const TCHAR* runners[] = {
    _T("ruby.exe"),
    NULL
};

const script_config config = { extensions, runners };
