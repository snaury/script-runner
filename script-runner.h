#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <signal.h>

typedef struct script_config {
    const TCHAR** extensions;
    const TCHAR** runners;
} script_config;

extern const script_config config;
