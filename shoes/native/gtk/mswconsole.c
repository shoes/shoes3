#ifndef GTK3
// fail only used for shoes_native_window_color will be deleted
#define GTK3
#endif
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/types/settings.h"
#ifdef SHOES_GTK_WIN32
// hat tip: https://justcheckingonall.wordpress.com/2008/08/29/console-window-win32-app/
#include <stdio.h>
#include <io.h>
#include <fcntl.h>


// called from main.c(skel) on Windows - works fine
static FILE* shoes_console_out = NULL;
static FILE* shoes_console_in = NULL;

int shoes_win32_console() {

    if (AllocConsole() == 0) {
        // cshoes.exe can get here
        printf("Already have console\n");
        return 0;
    }

    HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
    int hCrt = _open_osfhandle((intptr_t) handle_out, _O_TEXT);
    FILE* hf_out = _fdopen(hCrt, "w");
    setvbuf(hf_out, NULL, _IONBF, 1);
    *stdout = *hf_out;
    HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
    hCrt = _open_osfhandle((intptr_t) handle_in, _O_TEXT);
    FILE* hf_in = _fdopen(hCrt, "r");
    setvbuf(hf_in, NULL, _IONBF, 128);
    *stdin = *hf_in;

    //* stash handles
    shoes_console_out = hf_out;
    shoes_console_in = hf_in;
    return 1;
}

// Called by Shoes after ruby/gtk/shoes is initialized and running
void shoes_native_terminal(char *dir_path, int monitor, int columns, int row,
    int fontsize, char *fg, char *bg, char *title) {
    // has a console been setup by --console flag?
    if (shoes_console_out == NULL) {
        if (shoes_win32_console() == 0) // cshoes.exe can do this
            return;
    }
    // convert the (cached) FILE * for what ruby wants for fd[0], [1]...
    if (dup2(_fileno(shoes_console_out), 1) == -1)
        printf("failed dup2 of stdout\n");
    if (dup2(_fileno(shoes_console_out), 2) == -1)
        printf("failed dup2 of stderr\n");
    if (dup2(_fileno(shoes_console_in), 0) == -1)
        printf("failed dup2 of stdin\n");
    printf("created win32 console\n");
    return;
}

// For bug #428 
int shoes_win10_gtk3_22_check() {
    if (gtk_get_minor_version() < 22)
      return 0;
    // borrowed from
    // https://stackoverflow.com/questions/32115255/c-how-to-detect-windows-10
    int ret = 0;
    NTSTATUS(WINAPI *RtlGetVersion)(LPOSVERSIONINFOEXW);
    OSVERSIONINFOEXW osInfo;

    *(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll"), "RtlGetVersion");

    if (NULL != RtlGetVersion)
    {
        osInfo.dwOSVersionInfoSize = sizeof(osInfo);
        RtlGetVersion(&osInfo);
        ret = osInfo.dwMajorVersion;
    }
    //printf("windows version %i\n", ret); // win 7 returns '6' go figure
    return ret == 10;
}
#else
/*
int shoes_native_console(char *app_path)
{
  //printf("init gtk console\n");
  shoes_native_app_console(app_path);
  printf("gtk\010k\t console \t\tcreated\n"); //test \b \t in string
  return 1;
}
*/
#endif
