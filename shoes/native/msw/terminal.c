#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/native.h"
#include "shoes/types/color.h"
#include "shoes/internal.h"
#include "shoes/appwin32.h"
#include "shoes/native/windows.h"

#include <stdio.h>
#include <io.h>
#include <fcntl.h>


// Monitors 

void shoes_native_monitor_set(shoes_app *app) {
}

int shoes_native_monitor_get(shoes_app *app) {
  return 0;
}

int shoes_native_monitor_count(VALUE self) {
  return INT2NUM(1);
}

int shoes_native_monitor_default() {
  return 0;
}

void shoes_native_monitor_geometry(int idx, shoes_monitor_t *geo) {
}

/*
 * Console/terminal 
 * hat tip: https://justcheckingonall.wordpress.com/2008/08/29/console-window-win32-app/
 */

// called from main.c(skel) on Windows cmd line flag - per request. 
static FILE* shoes_console_out = NULL;
static FILE* shoes_console_in = NULL;

int shoes_win32_console() {

    if (AllocConsole() == 0) {
        // cshoes.exe can get here
        printf("Already have console\n");
        return 0;
    }

    HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
    int hCrt = _open_osfhandle((long) handle_out, _O_TEXT);
    FILE* hf_out = _fdopen(hCrt, "w");
    setvbuf(hf_out, NULL, _IONBF, 1);
    *stdout = *hf_out;
    HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
    hCrt = _open_osfhandle((long) handle_in, _O_TEXT);
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
