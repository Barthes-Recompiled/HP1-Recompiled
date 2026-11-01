/* xboxrecomp: one copy of the game at a time. hpps.exe holds a named mutex for its whole life; a second hpps.exe
 * while one runs says so, brings the running game's window forward and starts nothing. A second
 * copy would share saves\, shadercache\ and hpps_run.log with the first. */
#ifndef HPPS_SINGLE_INSTANCE_H
#define HPPS_SINGLE_INSTANCE_H

#include <wchar.h>
#include <windows.h>

#define HPPS_RUNNING_MUTEX L"Local\\hpps_game_running"
#define HPPS_GAME_WINDOW_CLASS "XboxRecompFramebuffer" /* fb_present.c */

/* the message, then the running game's window to the front (after the box: a fullscreen game would cover it) */
static inline void hpps_ReportAlreadyRunning(void)
{
    MessageBoxW(NULL, L"Harry Potter and the Philosopher's Stone is already running.",
                L"Harry Potter and the Philosopher's Stone", MB_OK | MB_ICONINFORMATION | MB_TOPMOST | MB_SETFOREGROUND);
    HWND w = FindWindowA(HPPS_GAME_WINDOW_CLASS, NULL);
    if (w) {
        if (IsIconic(w)) ShowWindow(w, SW_RESTORE);
        SetForegroundWindow(w);
    }
}

/* the game: takes the mutex for the rest of the process; 0 when another copy already has it */
static inline int hpps_ClaimSingleInstance(void)
{
    HANDLE m = CreateMutexW(NULL, FALSE, HPPS_RUNNING_MUTEX);
    return !(m && GetLastError() == ERROR_ALREADY_EXISTS);
}

/* whether a game holds the mutex now */
static inline int hpps_GameRunning(void)
{
    HANDLE m = OpenMutexW(SYNCHRONIZE, FALSE, HPPS_RUNNING_MUTEX);
    if (!m) return 0;
    CloseHandle(m);
    return 1;
}

#endif
