//
// main.c
// The generic launcher for Shoes.
//
#include <stdio.h>
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/internal.h"

#ifdef __APPLE__
#include <crt_externs.h>
#endif
#ifdef SHOES_GTK_WIN32
#define SHOES_WIN32
extern void shoes_win32_console();
extern void rb_w32_sysinit(int *, char ***);
#endif

#ifdef SHOES_WIN32
int WINAPI
WinMain(HINSTANCE inst, HINSTANCE inst2, LPSTR arg, int style)
#else
int
main(argc, argv)
  int argc;
  char *argv[];
#endif
{
  shoes_code code;
  char *path = NULL;
  int debug = 0;
/* TODO delete:
#ifdef __APPLE__
  char **env = *_NSGetEnviron();
#endif
*/
#ifdef SHOES_WIN32
  int argc;
  char **argv;
  //argc = shoes_win32_cmdvector(GetCommandLine(), &argv);
  // do we need the next two lines?
  //char *cmdline = GetCommandLine();
  //argv = CommandLineToArgvW(cmdline, &argc);
#endif


#ifdef SHOES_WIN32
  path = SHOE_ALLOC_N(char, SHOES_BUFSIZE);
  GetModuleFileName(NULL, (LPSTR)path, SHOES_BUFSIZE);
  rb_w32_sysinit(&argc, &argv);
#else
  path = argv[0];
#endif
  if (argc > 1 && strcmp(argv[1], "--ruby") == 0)
  {
    char bootup[SHOES_BUFSIZE];
    int len = shoes_snprintf(bootup,
      SHOES_BUFSIZE,
      "begin;"
        "DIR = File.expand_path(File.dirname(%%q<%s>));"
        "$:.replace([DIR+'/lib/ruby', DIR+'/lib', '.']);"
         "require 'shoes/envgem';"
        "DIR;"
      "rescue Object => e;"
        "puts(e.message);"
      "end",
      path);

    if (len < 0 || len >= SHOES_BUFSIZE)
      return 0;

    argc--;
    argv[1] = argv[0];
    argv = &argv[1];
    {
      RUBY_INIT_STACK
      /* in ruby 2.3+ we need to fake  */
      int zedc = 0;
      int *zeda = &zedc;
      ruby_sysinit(&zedc, &zeda);
      ruby_init();
      rb_eval_string(bootup);
      return ruby_run_node(ruby_options(argc, argv));
    }
  } 
  // Not --ruby argv
#ifdef SHOES_WIN32
  if (argc > 1 && strcmp(argv[1], "--console") == 0)
  {
    shoes_win32_console();
    // use the console just like a normal one - printf(), getchar(), ...
    // remove arg 1, compress argv
    int i;
    for (i=1; i< argc; i++) 
    {
      argv[i] = argv[i+1];
    }
    argv[i] = NULL;
    argc--;
    printf("Using new console: %s %s \n",argv[0], argv[1]);
 }
#endif
#ifdef SHOES_QUARTZ
  int osx_cshoes_launch = 0;
  if (argc > 1 && strcmp(argv[1], "--osx") == 0)
  {
    // only the cshoes script sets this flag. An open -a launch will not.
    osx_cshoes_launch = 1;  
    // remove arg 1, compress argv
    int i;
    for (i=1; i< argc; i++) 
    {
      argv[i] = argv[i+1];
    }
    argv[i] = NULL;
    argc--;
 }
#endif 
// check for the -d secret?  flag to invoke byebug in shoes.rb
// must be the first arg.
  if (argc > 1 && strcmp(argv[1], "-d") == 0)
  {
    debug = 1;  
    // remove arg 1, compress argv
    int i;
    for (i=1; i< argc; i++) 
    {
      argv[i] = argv[i+1];
    }
    argv[i] = NULL;
    argc--;
 }
#ifdef SHOES_WIN32
  code = shoes_init(inst, style, path);
#else
  code = shoes_init(path);
#endif
  fprintf(stderr, "[MAIN] shoes_init returned code=%d\n", code);
  if (code != SHOES_OK)
    goto done;
  
  fprintf(stderr, "[MAIN] Calling shoes_set_argv\n");
  shoes_set_argv(argc - 1, &argv[1]);
  
  fprintf(stderr, "[MAIN] Calling shoes_start\n");
  code = shoes_start(path, "/", debug);
  fprintf(stderr, "[MAIN] shoes_start returned code=%d\n", code);
  if (code != SHOES_OK) {
    fprintf(stderr, "[MAIN] code != SHOES_OK, going to done\n");
    goto done;
  }

done:
  fprintf(stderr, "[MAIN] At done label\n");
#ifdef SHOES_WIN32
  if (path != NULL)
    SHOE_FREE(path);
#endif
  fprintf(stderr, "[MAIN] Calling shoes_final\n");
  shoes_final();
  fprintf(stderr, "[MAIN] shoes_final returned\n");
  return 0;
}
