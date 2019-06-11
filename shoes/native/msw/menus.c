//
// Windows-specific menu code for Shoes.
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/native.h"
#include "shoes/types/color.h"
#include "shoes/internal.h"
#include "shoes/appwin32.h"
#include "shoes/types/menubar.h"
#include "shoes/types/menu.h"
#include "shoes/types/menuitem.h"
#include "shoes/types/settings.h"
#include "shoes/native/windows.h"
#include <commdlg.h>
#include <shlobj.h>


extern HACCEL shoes_win32_init_accels;

// Forward decls

WCHAR *shoes_win32_mi_create_str(shoes_menuitem *mi);
void shoes_win32_mi_redraw(shoes_menuitem *mi);

// This is called from a shoes script via types/menubar.c
// Returns a  Shoes menubar object but the natives are created
// later (or sooner?) in shoes_win32_menubar_setup
// WARNING! 
VALUE shoes_native_menubar_setup(shoes_app *app, void *nothing) {
  HMENU hMenubar = (HMENU)nothing;
  if (nothing == NULL) {
    //fprintf(stderr,"shoes_native_menubar_setup: creating mb\n");
    hMenubar = CreateMenu();
    app->os.menubar = hMenubar;
    app->have_menu = TRUE;
  } 
  if (NIL_P(app->menubar)) {
    // use the platform neutral calls to build Shoes menu
    // we can't do that for the menubar so we'll build it ourself
    VALUE mbv = shoes_menubar_alloc(cShoesMenubar);
    Get_TypedStruct2(mbv, shoes_menubar, mb);
    mb->context = app->canvas;
    mb->native = (void *)hMenubar;
    // save menubar object in app object
    app->menubar = mbv;
   }
  return app->menubar;
}
// A bit tricky in Windows. We create the native menus from inside the _
// message loop for 'hwnd'? Why? MAYBE NOT - app isn't set.
void shoes_win32_menubar_setup(shoes_app *app, HWND hwnd) {
  if (! app) {
    //fprintf(stderr, "shoes_win32_menubar_setup: app is nil\n");
    return;
  }
  shoes_menubar *mb;
  TypedData_Get_Struct(app->menubar, shoes_menubar, &shoes_menubar_type, mb);
  SetMenu(hwnd, (HMENU)mb->native);
  fprintf(stderr, "shoes_win32_menubar_setup: have an app!");
}

void shoes_native_build_menus(shoes_app *app, VALUE mbv) {
      // Shoes menu
      VALUE shoestext = rb_str_new2("File");
      Get_TypedStruct2(shoes_world->settings, shoes_settings, st);
      //VALUE shoestext = st->app_name;
      VALUE shoesmenu = shoes_menu_new(shoestext);
      int flags = MENUITEM_ENABLE;
      char *key = "";
      // New/open Shoes.show_selector
      VALUE otext = rb_str_new2("Open");
      VALUE oproc = rb_eval_string("proc { Shoes.show_selector }");
      VALUE oitem = shoes_menuitem_new(otext,flags|MENUITEM_CONTROL, "o", oproc, app->canvas);
      shoes_menu_append(shoesmenu, oitem);
      // -------- separator
      VALUE stext = rb_str_new2("--- a seperator");
      VALUE s1item = shoes_menuitem_new(stext, flags, key, Qnil, app->canvas);
      shoes_menu_append(shoesmenu, s1item);
      // Manual 
      VALUE mtext = rb_str_new2("Manual");
      VALUE mproc = rb_eval_string("proc { Shoes.show_manual }");
      VALUE mitem = shoes_menuitem_new(mtext, flags, key, mproc, app->canvas);
      shoes_menu_append(shoesmenu, mitem);
      // Cobbler
      VALUE ctext = rb_str_new2("Cobbler");
      VALUE cproc = rb_eval_string("proc { Shoes.cobbler }");
      VALUE citem = shoes_menuitem_new(ctext, flags, key, cproc, app->canvas);
      shoes_menu_append(shoesmenu, citem);

      // Profile - bug in profiler (#400 , @dredknight)
      VALUE ftext = rb_str_new2("Profile");
      VALUE fproc = rb_eval_string("proc { require 'shoes/profiler'; Shoes.profile(nil) }");
      VALUE fitem = shoes_menuitem_new(ftext, flags, key, fproc, app->canvas);
      shoes_menu_append(shoesmenu, fitem);
      // Package
      VALUE ptext = rb_str_new2("Package");
      VALUE pproc = rb_eval_string("proc { Shoes.app_package }");
      VALUE pitem = shoes_menuitem_new(ptext, flags, key, pproc, app->canvas);
      shoes_menu_append(shoesmenu, pitem);
      // --------
      VALUE s2item = shoes_menuitem_new(stext, flags, key, Qnil, app->canvas);
      shoes_menu_append(shoesmenu, s2item);

      // Quit
      VALUE qtext = rb_str_new2("Quit");
      VALUE qproc = rb_eval_string("proc { Shoes.quit }");
      VALUE qitem = shoes_menuitem_new(qtext, flags | MENUITEM_CONTROL, "q", qproc, app->canvas); 

      shoes_menu_append(shoesmenu, qitem);
      shoes_menubar_append(mbv, shoesmenu);
}

// ---- mapping from int(id) to *menuitem ---------

static int shoes_win32_menuctr = 1;
static void *shoes_win32_menutbl[256] = {0};

int shoes_win32_menuid(shoes_menuitem *mi) {
  int newi = shoes_win32_menuctr++;
  shoes_win32_menutbl[newi] = (void *)mi;
  return newi;
}
// Called from the message loop - we may have to search the menu tree
// to find the matching menuitem_number
void shoes_win32_menu_lookup(shoes_app *app, int menuitem_number) {
  shoes_menuitem *mi;
  if (menuitem_number >= shoes_win32_menuctr) {
    fprintf(stderr, "Out of range menu id: %d\n", menuitem_number);
    return;
  }
  mi = shoes_win32_menutbl[menuitem_number];
  if (mi) {
    fprintf(stderr, "Dispatch %d %s\n", mi->extra, mi->title);
    shoes_safe_block(mi->context, mi->block, rb_ary_new3(1, mi->context));
  }
}

// --------- remember, we just want the natives to match what's already
//           been done to the Shoes objects.

void shoes_native_menubar_append(shoes_menubar *mb, shoes_menu *mn) {
  HMENU menubar = (HMENU)mb->native;
  HMENU menu = (HMENU)mn->native;
  shoes_canvas *canvas;
  TypedData_Get_Struct(mb->context, shoes_canvas, &shoes_canvas_type, canvas);
  shoes_app *app = canvas->app;
  WCHAR *wstr = shoes_wchar(mn->title);
  AppendMenuW(menubar, MF_POPUP, (UINT_PTR) menu, wstr);
  BOOL rtn = DrawMenuBar(app->slot->window);
  char errstr[20];
  if (rtn == 0)
    rb_raise(rb_eArgError,"Menubar append of %s failed: %d\n", mn->title, GetLastError());
}

void shoes_native_menubar_insert(shoes_menubar *mb, shoes_menu *mn, int pos) {
  int err;
  err = InsertMenuW((HMENU)mb->native, pos, 
      (MF_POPUP|MF_BYPOSITION | MF_ENABLED | MF_STRING),
      (UINT_PTR)mn->native, shoes_wchar(mn->title));  
  if (err == 0) {
    rb_raise(rb_eArgError,"Failed to insert into menubar: %d", GetLastError());
  }
  if (mb->context != Qnil) {
    shoes_canvas *canvas;
    TypedData_Get_Struct(mb->context, shoes_canvas, &shoes_canvas_type, canvas);
    shoes_app *app = canvas->app;
    DrawMenuBar(app->slot->window);
  }
}

void shoes_native_menubar_remove(shoes_menubar *mb, int pos) {
  int err;
  err = DeleteMenu((HMENU)mb->native, pos, MF_BYPOSITION);
  if (err == 0) {
    rb_raise(rb_eArgError,"Failed to remove menu: %d", GetLastError());
  }
  if (mb->context != Qnil) {
    shoes_canvas *canvas;
    TypedData_Get_Struct(mb->context, shoes_canvas, &shoes_canvas_type, canvas);
    shoes_app *app = canvas->app;
    DrawMenuBar(app->slot->window);
  }
}

void *shoes_native_menu_new(shoes_menu *mn) {
  HMENU menu = CreateMenu();
  mn->native = (void *)menu; 
  mn->extra = (void *)menu;
  //fprintf(stderr, "menu new: %s\n", mn->title);
  return mn->native;
}

void shoes_native_menu_append(shoes_menu *mn, shoes_menuitem *mi) {
  HMENU menu = (HMENU)mn->native;
  HMENU menuitem = (HMENU)mi->native;
  if (mi->extra == -1) {
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
  } else {
    AppendMenuW(menu, MF_STRING, mi->extra, shoes_win32_mi_create_str(mi));
    // Wacky: enable or disable the menuitem in a separate call. Why?
    if (mi->state & MENUITEM_ENABLE)
      EnableMenuItem(menu, mi->extra, MF_BYCOMMAND | MF_ENABLED);
    else
      EnableMenuItem(menu, mi->extra, MF_BYCOMMAND | MF_GRAYED);
    //fprintf(stderr, "menu_append: %s << %s\n", mn->title, mi->title);
  }
}

void shoes_native_menu_insert(shoes_menu *mn, shoes_menuitem *mi, int pos) {
  // create an MENUITEMINFO from a shoes_menuitem.
  MENUITEMINFO *wmi = malloc(sizeof(MENUITEMINFO));
  memset(wmi,'\0',sizeof(MENUITEMINFO));
  wmi->cbSize = sizeof(MENUITEMINFO);
  wmi->fMask = MIIM_FTYPE;
  if (mi->extra == -1) {
    wmi->fType | MFT_SEPARATOR;
  } else {
    wmi->fMask |= (MIIM_ID | MIIM_STRING | MIIM_STATE);
    wmi->fType = MFT_STRING;
    wmi->dwTypeData = (LPSTR)shoes_win32_mi_create_str(mi);
    wmi->wID = mi->extra;
    wmi->fState = (mi->state & MENUITEM_ENABLE) ? MFS_ENABLED : MFS_GRAYED;
  }
  // TODO unclear if Windows copies the item. LPCMENUITEMINFOW cast seems wrong
  InsertMenuItemW((HMENU)mn->native, pos, true, (LPCMENUITEMINFOW)wmi);
  // Trigger redraw
  shoes_win32_mi_redraw(mi);
}

void shoes_native_menu_remove(shoes_menu *mn, int pos) {
  int err;
  err = DeleteMenu((HMENU)mn->native, pos, MF_BYPOSITION);
  if (err == 0) {
    rb_raise(rb_eArgError,"Failed to remove menuitem: %d", GetLastError());
  }
  // redraw menuber
  if ((VALUE)mn->context != Qnil) {
    shoes_canvas *canvas;
    TypedData_Get_Struct((VALUE)mn->context, shoes_canvas, &shoes_canvas_type, canvas);
    DrawMenuBar(canvas->app->os.window);
  }
}

WCHAR* shoes_win32_mi_create_str(shoes_menuitem *mi) {
  char tmp[200];
  strcpy(tmp, mi->title);
  int pos = strlen(tmp);   
  if (mi->state & (MENUITEM_CONTROL | MENUITEM_ALT | MENUITEM_SHIFT)) {
    strcat(tmp,"\t");
    if (mi->state & MENUITEM_SHIFT)
      strcat(tmp, "Shift+");
    if (mi->state & MENUITEM_ALT)
      strcat(tmp, "Alt+");
    if (mi->state & MENUITEM_CONTROL)
      strcat(tmp, "Ctrl+");
    strcat(tmp, strupr(mi->key));
  }
  return shoes_wchar(tmp);
}

void shoes_win32_setaccel(ACCEL *nacc, VALUE canvas) {
  // Find the app that has the canvas (shoes_menuitem.context)
  shoes_app *app = NULL;
  shoes_canvas *cvs;
  TypedData_Get_Struct(canvas, shoes_canvas, &shoes_canvas_type, cvs);
  app = cvs->app;
  if (app == NULL) {
    fprintf(stderr, "No app for accelerator?\n");
    return;
  }
#if 0 // debugging - use resource based table. WORKS.
  if (app->os.accelH)
    DestroyAcceleratorTable(app->os.accelH);
  app->os.acc_cnt = 1;
  ACCEL tbl[1];
  CopyAcceleratorTable(shoes_win32_init_accels, tbl, 1);
  app->os.accelH = CreateAcceleratorTable(tbl, app->os.acc_cnt);
  return;
#endif
  LPACCEL tmpP;
  int cnt = 0;
  if (app->os.accelH)
    cnt = CopyAcceleratorTable(app->os.accelH, NULL, 0);
  tmpP = (LPACCEL) LocalAlloc(LPTR, (cnt + 1) * sizeof(ACCEL)); 
  CopyAcceleratorTable(app->os.accelH, tmpP, cnt);
#if 0
  tmpP[cnt].fVirt = nacc->fVirt;
  tmpP[cnt].key = nacc->key;
  tmpP[cnt].cmd = nacc->cmd;
#else
  tmpP[cnt] = *nacc;
#endif
  cnt++;
  DestroyAcceleratorTable(app->os.accelH);
  app->os.accelH = CreateAcceleratorTable(tmpP, cnt);
  app->os.acc_cnt = cnt; //unused ?
}

void *shoes_native_menuitem_new(shoes_menuitem *mi) {
  HMENU gmi;
  gmi = CreateMenu();
  mi->native = (void *)gmi;
  mi->extra = shoes_win32_menuid(mi); // Get a new id
  if (mi->state & (MENUITEM_SHIFT | MENUITEM_CONTROL | MENUITEM_ALT)) {
    // need to (re)create an ACCEL table to include this item
    ACCEL nacc;
    nacc.fVirt = FVIRTKEY;
    if (mi->state & MENUITEM_CONTROL)
      nacc.fVirt |= FCONTROL;
    if (mi->state & MENUITEM_ALT)
      nacc.fVirt |= FALT;
    if (mi->state & MENUITEM_SHIFT)
      nacc.fVirt |= FSHIFT;
    unsigned char c = mi->key[0];
    c = toupper(c); // TODO: NOT UTF-8 or 16 
    nacc.key = c;  // it's a WORD
    nacc.cmd = mi->extra;
    shoes_win32_setaccel(&nacc, mi->context);
  }
  fprintf(stderr, "mi new: %d => %s\n", mi->extra, mi->title);
  return (void *)mi->native;
}

void *shoes_native_menusep_new(shoes_menuitem *mi) {
  HMENU gmi;
  gmi = CreateMenu();
  mi->native = (void *)gmi;
  mi->extra = -1;    // Windows API is not a friendly creature
  return (void *)mi->native;
}

void shoes_win32_mi_redraw(shoes_menuitem *mi) {
    VALUE micxt = mi->context;
    shoes_canvas *canvas;
    TypedData_Get_Struct(micxt, shoes_canvas, &shoes_canvas_type, canvas);
    DrawMenuBar(canvas->app->os.window);
}

int shoes_win32_menu_pos(shoes_menuitem *mi) {
  shoes_menu *menu = mi->parent;
  if (menu) {
    int cnt = RARRAY_LEN(menu->items);
    // Find the position (0..n)
    int pos;
    for (pos = 0; pos < cnt; pos++) {
      VALUE miv = rb_ary_entry(menu->items, pos);
      shoes_menuitem *mivl;
      TypedData_Get_Struct(miv, shoes_menuitem, &shoes_menuitem_type, mivl);
      if (mivl == mi)
        return pos;
    }
  }
  return -1;
}

void shoes_native_menuitem_enable(shoes_menuitem *mi, int ns) {
  shoes_menu *menu = mi->parent;
  int olds;
  if (menu) {
    int pos = shoes_win32_menu_pos(mi);
    if (pos < 0) 
      rb_raise(rb_eArgError, "could not find menuitem to enable");
    if (ns)
      olds = EnableMenuItem((HMENU)menu->native, pos, MF_BYPOSITION | MF_ENABLED);
    else
      olds = EnableMenuItem((HMENU)menu->native, pos, MF_BYPOSITION | MF_GRAYED);
    mi->state = ns;
    shoes_win32_mi_redraw(mi);
    //printf("EnableMenuItem rtn: %x\n", olds);
  } else {
    // unattached menuitem. 
    mi->state = ns;
  }
}

void shoes_native_menuitem_set_title(shoes_menuitem *mi) {
  shoes_menu *menu = mi->parent;
  int pos = shoes_win32_menu_pos(mi);
  if (pos < 0)
    rb_raise(rb_eArgError, "could not find menuitem to set");
  int err;
  char buf[80];
  if (mi->state & (MENUITEM_SHIFT | MENUITEM_CONTROL | MENUITEM_ALT)) {
    // we need to deal with the damn accelerator in the native title.
    strcpy(buf, mi->title);
    strcat(buf,"\t");
    if (mi->state & MENUITEM_SHIFT) 
      strcat(buf,"Shift+");
    if (mi->state & MENUITEM_CONTROL) 
      strcat(buf,"Ctrl+");
    if (mi->state & MENUITEM_ALT) 
      strcat(buf,"Alt+");
    strcat(buf, strupr(mi->key));   // assumes null term string.
  } else {
    strcpy(buf, mi->title);
  }
  // ModifyMenuW works
  err = ModifyMenuW((HMENU)menu->native, pos, MF_BYPOSITION | MF_STRING,
      0, shoes_wchar(buf));
  if (err == 0)
    rb_raise(rb_eArgError, "Windows failed to update title, code: %d", GetLastError());
  err = EnableMenuItem((HMENU)menu->native, pos, 
      MF_BYPOSITION | (mi->state | MENUITEM_ENABLE ? MF_ENABLED: MF_GRAYED));
  shoes_win32_mi_redraw(mi);
}

void shoes_native_menuitem_set_key(shoes_menuitem *mi, int newflags, char *newkey) {
  // This needs to change the title AND change the accelerator table
  shoes_app *app = NULL;
  shoes_canvas *cvs;
  TypedData_Get_Struct(mi->context, shoes_canvas, &shoes_canvas_type, cvs);
  app = cvs->app;
  if (app == NULL) {
    fprintf(stderr, "No app for modified accelerator?\n");
    return;
  }  
  WORD oldkey = (WORD)mi->key[0];
  mi->state = newflags;
  if (mi->key) {
    free(mi->key);
    mi->key = strdup(strupr(newkey));
  }
  // find and replace accel  
  LPACCEL tmpP;
  int cnt = 0;
  if (app->os.accelH)
    cnt = CopyAcceleratorTable(app->os.accelH, NULL, 0);
  if (cnt > 0) {
    tmpP = (LPACCEL) LocalAlloc(LPTR, cnt * sizeof(ACCEL)); 
    CopyAcceleratorTable(app->os.accelH, tmpP, cnt);
    int i;
    for (i = 0; i < cnt; i++) {
      if (tmpP[i].key == oldkey) {
        tmpP[i].key = (WORD)newkey[0];
        tmpP[i].fVirt = FVIRTKEY;
        if (newflags & MENUITEM_CONTROL)
          tmpP[i].fVirt |= FCONTROL;
        if (newflags & MENUITEM_ALT)
          tmpP[i].fVirt |= FALT;
        if (newflags & MENUITEM_SHIFT)
          tmpP[i].fVirt |= FSHIFT;
        break;
      }
    }
    if (i <= cnt) {
      DestroyAcceleratorTable(app->os.accelH);
      app->os.accelH = CreateAcceleratorTable(tmpP, cnt);
    }
  }
  // now update the visual menu string. Also redraws menubar.
  shoes_native_menuitem_set_title(mi);
}

