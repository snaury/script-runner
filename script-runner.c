/*
    Copyright (c) 2010, Alexey Borzenkov <snaury@gmail.com>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name of the author nor the names of its contributors
          may be used to endorse or promote products derived from this
          software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "script-runner.h"

/**
 * skip_program_name
 *
 * Given command line p skips over the program name and
 * returns pointer to the first argument in p
 */
const TCHAR* skip_program_name(const TCHAR* p)
{
    int dq = 0;
    while (*p > _T(' ') || (*p && dq)) {
        if (*p == _T('"'))
            dq = !dq;
        ++p;
    }
    while (*p && *p <= _T(' '))
        ++p;
    return p;
}

/**
 * skip_dirname
 *
 * Given path p skips over dirname and
 * returns pointer to filename in p
 */
const TCHAR* skip_dirname(const TCHAR* p)
{
    const TCHAR* p1 = _tcsrchr(p, _T('\\'));
    const TCHAR* p2 = _tcsrchr(p, _T('/'));
    const TCHAR* pend = p1 > p2 ? p1 : p2;
    if (!pend)
        return p; /* no dirname */
    while (*pend && *pend == _T('\\') || *pend == _T('/'))
        ++pend;
    return pend; /* pend points to the filename */
}

/**
 * get_file_ext
 * Given path p skips over filename and
 * returns pointer to file extension
 */
const TCHAR* get_file_ext(const TCHAR* p)
{
    const TCHAR* pext;
    p = skip_dirname(p);
    pext = _tcsrchr(p, _T('.'));
    if (!pext)
        return p + _tcslen(p); /* no extension*/
    return pext;
}

/**
 * get_module_filename
 *
 * returns filename of module hInstance
 * returned string must be freed using free()
 */
TCHAR* get_module_filename(HINSTANCE hInstance)
{
    TCHAR buffer[MAX_PATH+1];
    buffer[0] = _T('\0');
    GetModuleFileName(hInstance, buffer, MAX_PATH);
    buffer[MAX_PATH] = _T('\0');
    return _tcsdup(buffer);
}

/**
 * dirname
 *
 * returns dirname of path p
 * returned string must be freed using free()
 */
TCHAR* dirname(const TCHAR* p)
{
    TCHAR* r;
    const TCHAR* p1 = _tcsrchr(p, _T('\\'));
    const TCHAR* p2 = _tcsrchr(p, _T('/'));
    const TCHAR* pend = p1 > p2 ? p1 : p2;
    if (!pend)
        return _tcsdup(_T(""));
    r = malloc((pend - p + 1) * sizeof(TCHAR));
    memcpy(r, p, (pend - p) * sizeof(TCHAR));
    r[pend - p] = '\0';
    return r;
}

/**
 * join
 *
 * returns join of paths p1 and p2
 * returned string must be freed using free()
 */
TCHAR* join(const TCHAR* p1, const TCHAR* p2)
{
    TCHAR* r;
    int p1len, p2len;
    const TCHAR* p1end = p1 + _tcslen(p1);
    const TCHAR* p2end = p2 + _tcslen(p2);
    if (p1 == p1end)
        return _tcsdup(p2);
    if (p2 == p2end)
        return _tcsdup(p1);
    while (p1 != p1end && (p1end[-1] == _T('\\') || p1end[-1] == _T('/')))
        --p1end;
    while (p2 != p2end && (p2[0] == _T('\\') || p2[0] == _T('/')))
        ++p2;
    p1len = p1end - p1;
    p2len = p2end - p2;
    r = malloc((p1len + 1 + p2len + 1) * sizeof(TCHAR));
    memcpy(r, p1, p1len * sizeof(TCHAR));
    r[p1len] = _T('\\');
    memcpy(r + p1len + 1, p2, p2len * sizeof(TCHAR));
    r[p1len + 1 + p2len] = _T('\0');
    return r;
}

/**
 * HasConsole
 * Taken from Tcl sources
 */
static BOOL HasConsole()
{
    HANDLE handle;

    handle = CreateFile(_T("CONOUT$"), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
	return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * run
 * Based on Tcl sources
 *
 * runs command specified in command
 * returns 0 on success, non-zero on failure
 * on success orc will be filled with exit-code of the process
 */
static int run(TCHAR* command, int* orc)
{
    HANDLE hProcess;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD status;

    hProcess = GetCurrentProcess();
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags    = STARTF_USESTDHANDLES;
    si.hStdInput  = INVALID_HANDLE_VALUE;
    si.hStdOutput = INVALID_HANDLE_VALUE;
    si.hStdError  = INVALID_HANDLE_VALUE;
    DuplicateHandle(hProcess, GetStdHandle(STD_INPUT_HANDLE),  hProcess, &si.hStdInput,  0, TRUE, DUPLICATE_SAME_ACCESS);
    DuplicateHandle(hProcess, GetStdHandle(STD_OUTPUT_HANDLE), hProcess, &si.hStdOutput, 0, TRUE, DUPLICATE_SAME_ACCESS);
    DuplicateHandle(hProcess, GetStdHandle(STD_ERROR_HANDLE),  hProcess, &si.hStdError,  0, TRUE, DUPLICATE_SAME_ACCESS);

    if (!CreateProcess(NULL, command, NULL, NULL, TRUE, HasConsole() ? 0 : DETACHED_PROCESS, NULL, NULL, &si, &pi))
        return -1;

    WaitForInputIdle(pi.hProcess, 5000);
    CloseHandle(pi.hThread);
    if (si.hStdInput != INVALID_HANDLE_VALUE)  CloseHandle(si.hStdInput);
    if (si.hStdOutput != INVALID_HANDLE_VALUE) CloseHandle(si.hStdOutput);
    if (si.hStdError != INVALID_HANDLE_VALUE)  CloseHandle(si.hStdError);

    WaitForSingleObject(pi.hProcess, INFINITE);
    if (!GetExitCodeProcess(pi.hProcess, &status)) {
        CloseHandle(pi.hProcess);
        return -1;
    }
    CloseHandle(pi.hProcess);
    if (orc)
        *orc = (int)status;
    return 0;
}

/**
 * try_script
 *
 * given script name and arguments args
 * tries to run it using available runners
 * returns 0 on success, non-zero on failure
 * on success orc will be filled with exit-code of the process
 */
static int try_script(const TCHAR* name, const TCHAR* args, int* orc)
{
    int rc = -1;
    DWORD attr;
    const TCHAR* p;
    int needquotes = 0;
    TCHAR* quotedname = NULL;
    size_t maxlen = 0;
    TCHAR* cmdline;
    const TCHAR** runner;

    attr = GetFileAttributes(name);
    if (attr == INVALID_FILE_ATTRIBUTES ||
        attr & FILE_ATTRIBUTE_DIRECTORY)
        return -1; /* we can't execute non-existent file or directory */

    for (p = name; *p; ++p) {
        if (*p <= _T(' ') || *p == '&') {
            needquotes = 1;
            break;
        }
    }

    if (needquotes) {
        /* since name is a filename quoting is generally simple */
        quotedname = malloc((1 + _tcslen(name) + 2) * sizeof(TCHAR));
        _stprintf(quotedname, _T("\"%s\""), name);
    }

    /* find maximum length of runner names */
    for (runner = config.runners; *runner; ++runner) {
        size_t len = _tcslen(*runner);
        if (maxlen < len)
            maxlen = len;
    }

    cmdline = malloc((maxlen +
                      1 + _tcslen(quotedname ? quotedname : name) +
                      1 + _tcslen(args) + 1) * sizeof(TCHAR));

    /* try executing every runner in order */
    for (runner = config.runners; *runner; ++runner) {
        _stprintf(cmdline, _T("%s %s %s"), *runner, quotedname ? quotedname : name, args);
        if (!run(cmdline, orc)) {
            rc = 0;
            break;
        }
    }

    if (cmdline)
        free(cmdline);
    if (quotedname)
        free(quotedname);

    return rc;
}

int main(int argc, char** argv)
{
    int rc = 127;
    HINSTANCE hInstance = GetModuleHandle(NULL);
    const TCHAR* args = skip_program_name(GetCommandLine());
    TCHAR* modulename = get_module_filename(hInstance);
    TCHAR* moduleext = (TCHAR*)get_file_ext(modulename);
    size_t maxlen = 0;
    const TCHAR** ext;
    TCHAR* name;
    TCHAR* nameext;
    int found = 0;

    for (ext = config.extensions; *ext; ++ext) {
        size_t len = _tcslen(*ext);
        if (maxlen < len)
            maxlen = len;
    }

    name = malloc(((moduleext - modulename) + maxlen + 1) * sizeof(TCHAR));
    _tcsncpy(name, modulename, moduleext - modulename);
    nameext = name + (moduleext - modulename);

    signal(SIGINT, SIG_IGN); /* disable Ctrl+C */
    for (ext = config.extensions; *ext; ++ext) {
        _tcscpy(nameext, *ext);
        if (!try_script(name, args, &rc)) {
            found = 1;
            break;
        }
    }
    signal(SIGINT, SIG_DFL); /* enable Ctrl+C */

    if (!found)
        _ftprintf(stderr, _T("Error: stub could not find or execute the script file\n"));

    free(name);
    free(modulename);
    return rc;
}
