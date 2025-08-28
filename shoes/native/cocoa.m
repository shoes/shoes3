//
// shoes/native-cocoa.m
// ObjC Cocoa-specific code for Shoes.
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/types.h"
#include "shoes/native/cocoa/button.h" // needed? 
#include "shoes/native/cocoa/textview.h"
#include "shoes/types/event.h"
#include "shoes/types/menubar.h"
#include "shoes/types/menu.h"
#include "shoes/types/menuitem.h"
#include "shoes/native/cocoa/menus.h"

extern VALUE cTimer;

#import <Carbon/Carbon.h>
#import <CoreText/CoreText.h>

#define HEIGHT_PAD 6

#define INIT    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]
#define RELEASE [pool release]
#define COCOA_DO(statements) do {\
  INIT; \
  @try { statements; } \
  @catch (NSException *e) { ; } \
  RELEASE; \
} while (0)

extern void shoes_osx_stdout_sink(); // in cocoa-term.m
int win_current_tmo = 10; // unused in OSX

@implementation ShoesEvents
- (id)init
{
  if ((self = [super init]))
    count = 0;
  return self;
}

- (void)idle: (NSTimer *)t
{
  // Safety check for shoes_world
  if (!shoes_world) return;
  
  if (count < 100)
  {
    count++;
    // Check shoes_world->apps is valid before accessing
    if (count == 100 && shoes_world->apps && RARRAY_LEN(shoes_world->apps) == 0)
      rb_eval_string("Shoes.splash");
  }
  // Don't call Ruby sleep in the event loop - it can cause issues
  // rb_eval_string("sleep(0.001)");
}

- (BOOL) application: (NSApplication *) anApplication
    openFile: (NSString *) aFileName
{
  // if launched from terminal (cshoes), DON'T allow this duplicated 
  if (! osx_cshoes_launch) {
    shoes_load([aFileName UTF8String]);
  }
  return YES;
}

- (void)openFile: (id)sender
{
  rb_eval_string("Shoes.show_selector");
}
- (void)package: (id)sender
{
  rb_eval_string("Shoes.make_pack");
}
- (void)showLog: (id)sender
{
  rb_eval_string("Shoes.show_log");
}
- (void)emulateKey: (NSString *)key modifierFlags: (unsigned int)flags withoutModifiers: (NSString *)key2
{
  ShoesWindow *win = (ShoesWindow *)[NSApp keyWindow];
  [win keyDown: [NSEvent keyEventWithType:NSKeyDown
    location:NSMakePoint(0,0) modifierFlags:flags
    timestamp:0 windowNumber:0 context:nil
    characters:key charactersIgnoringModifiers:key2 isARepeat:NO
    keyCode:0]];
}
- (void)help: (id)sender
{
  rb_eval_string("Shoes.show_manual");
}
- (void)undo: (id)sender
{
  [self emulateKey: @"z" modifierFlags: NSCommandKeyMask withoutModifiers: @"z"];
}
- (void)redo: (id)sender
{
  [self emulateKey: @"Z" modifierFlags: NSCommandKeyMask|NSShiftKeyMask withoutModifiers: @"z"];
}
- (void)cut: (id)sender
{
  [self emulateKey: @"x" modifierFlags: NSCommandKeyMask withoutModifiers: @"x"];
}
- (void)copy: (id)sender
{
  [self emulateKey: @"c" modifierFlags: NSCommandKeyMask withoutModifiers: @"c"];
}
- (void)paste: (id)sender
{
  [self emulateKey: @"v" modifierFlags: NSCommandKeyMask withoutModifiers: @"v"];
}
- (void)selectAll: (id)sender
{
  [self emulateKey: @"a" modifierFlags: NSCommandKeyMask withoutModifiers: @"a"];
}
- (void) menuTrigger: (id)sender
{
  ShoesMenuItem *wr = [sender representedObject];
  shoes_menuitem *mi = wr->wrapped;
  // We have to find 'context' via shoes_world->apps[]
  if (mi->context == Qnil) {
    VALUE appv = rb_ary_entry(shoes_world->apps, 0);
    // Do we have an app (window?) - use it's context
    if (!NIL_P(appv)) {
			shoes_app *app;
			Data_Get_Struct(appv, shoes_app, app);
			mi->context = app->canvas;
		}
  }
  //fprintf(stderr, "menu %s triggered\n", mi->title);
  shoes_safe_block(mi->context, mi->block, rb_ary_new3(1, mi->context));
}
@end

@implementation ShoesWindow
- (void)prepareWithApp: (VALUE)a
{
  app = a;
  //[self center];
  [self makeKeyAndOrderFront: self];
  [self setAcceptsMouseMovedEvents: YES];
  [self setAutorecalculatesKeyViewLoop: YES];
  [self setDelegate: (id <NSWindowDelegate>)self];
}
- (void)disconnectApp
{
  app = Qnil;
}
- (void)sendMotion: (NSEvent *)e ofType: (ID)type withButton: (int)b
{
  shoes_app *a;
  shoes_canvas *canvas;
  int modify = 0;
  if ([e modifierFlags] & NSShiftKeyMask)
    modify = modify | SHOES_MODIFY_SHIFT;
  if ([e modifierFlags] & NSControlKeyMask)
    modify = modify | SHOES_MODIFY_CTRL;
  /* platform and theme specific 
  if ([e.modifierFlags] & NSAlternateKeyMask)
    altkey = 1;
  if ([e.modifierFlags] & NSCommandKeyMask)
    key = 1;   
  */ 
  NSPoint p = [e locationInWindow];
  Data_Get_Struct(app, shoes_app, a);
  Data_Get_Struct(a->canvas, shoes_canvas, canvas);
  if (type == s_motion)
    shoes_app_motion(a, ROUND(p.x), (canvas->height - ROUND(p.y)) + canvas->slot->scrolly, modify);
  else if (type == s_click)
    shoes_app_click(a, b, ROUND(p.x), (canvas->height - ROUND(p.y)) + canvas->slot->scrolly, modify);
  else if (type == s_release)
    shoes_app_release(a, b, ROUND(p.x), (canvas->height - ROUND(p.y)) + canvas->slot->scrolly,  modify);
}
- (void)mouseDown: (NSEvent *)e
{
  [self sendMotion: e ofType: s_click withButton: 1];
}
- (void)rightMouseDown: (NSEvent *)e
{
  [self sendMotion: e ofType: s_click withButton: 2];
}
- (void)otherMouseDown: (NSEvent *)e
{
  [self sendMotion: e ofType: s_click withButton: 3];
}
- (void)mouseUp: (NSEvent *)e
{
  [self sendMotion: e ofType: s_release withButton: 1];
}
- (void)rightMouseUp: (NSEvent *)e
{
  [self sendMotion: e ofType: s_release withButton: 2];
}
- (void)otherMouseUp: (NSEvent *)e
{
  [self sendMotion: e ofType: s_release withButton: 3];
}
- (void)mouseMoved: (NSEvent *)e
{
  [self sendMotion: e ofType: s_motion withButton: 0];
}
- (void)mouseDragged: (NSEvent *)e
{
  [self sendMotion: e ofType: s_motion withButton: 0];
}
- (void)rightMouseDragged: (NSEvent *)e
{
  [self sendMotion: e ofType: s_motion withButton: 0];
}
- (void)otherMouseDragged: (NSEvent *)e
{
  [self sendMotion: e ofType: s_motion withButton: 0];
}
- (void)scrollWheel: (NSEvent *)e
{
  ID wheel;
  CGFloat dy = [e deltaY];
  NSPoint p = [e locationInWindow];
  shoes_app *a;

  if (dy == 0)
    return;
  else if (dy > 0)
    wheel = s_up;
  else
  {
    wheel = s_down;
    dy = -dy;
  }
  int modify = 0;
  if ([e modifierFlags] & NSShiftKeyMask)
    modify = modify | SHOES_MODIFY_SHIFT;
  if ([e modifierFlags] & NSControlKeyMask)
    modify = modify | SHOES_MODIFY_CTRL;
  Data_Get_Struct(app, shoes_app, a);
  for (; dy > 0.; dy--)
    shoes_app_wheel(a, wheel, ROUND(p.x), ROUND(p.y), modify);
}

- (void)keyDown: (NSEvent *)e
{
  shoes_app *a;
  VALUE v = Qnil;
  NSUInteger modifier = [e modifierFlags];
  unsigned short key = [e keyCode];
  INIT;

  Data_Get_Struct(app, shoes_app, a);
  KEY_SYM(ESCAPE, escape)
  KEY_SYM(INSERT, insert)
  KEY_SYM(DELETE, delete)
  KEY_SYM(TAB, tab)
  KEY_SYM(BS, backspace)
  KEY_SYM(PRIOR, page_up)
  KEY_SYM(NEXT, page_down)
  KEY_SYM(HOME, home)
  KEY_SYM(END, end)
  KEY_SYM(LEFT, left)
  KEY_SYM(UP, up)
  KEY_SYM(RIGHT, right)
  KEY_SYM(DOWN, down)
  KEY_SYM(F1, f1)
  KEY_SYM(F2, f2)
  KEY_SYM(F3, f3)
  KEY_SYM(F4, f4)
  KEY_SYM(F5, f5)
  KEY_SYM(F6, f6)
  KEY_SYM(F7, f7)
  KEY_SYM(F8, f8)
  KEY_SYM(F9, f9)
  KEY_SYM(F10, f10)
  KEY_SYM(F11, f11)
  KEY_SYM(F12, f12)
  {
    NSString *str = [e charactersIgnoringModifiers];
    if (str)
    {
      char *utf8 = [str UTF8String];
      if (utf8[0] == '\r' && [str length] == 1)
        v = rb_str_new2("\n");
      else
        v = rb_str_new2(utf8);
    }
  }

  if (SYMBOL_P(v))
  {
    if ((modifier & NSCommandKeyMask) || (modifier & NSAlternateKeyMask))
      KEY_STATE(alt);
    if (modifier & NSShiftKeyMask)
      KEY_STATE(shift);
    if (modifier & NSControlKeyMask)
      KEY_STATE(control);
  }
  else
  {
    if ((modifier & NSCommandKeyMask) || (modifier & NSAlternateKeyMask))
      KEY_STATE(alt);
  }

  if (v != Qnil)
  {
    shoes_app_keypress(a, v);
  }
  RELEASE;
}
- (BOOL)canBecomeKeyWindow
{
  return YES;
}
- (BOOL)canBecomeMainWindow
{
  return YES;
}
- (void)windowWillClose: (NSNotification *)n
{
  if (!NIL_P(app)) {
    shoes_app *a;
    Data_Get_Struct(app, shoes_app, a);
    shoes_app_remove(a);
  }
}
@end

@implementation ShoesView
- (id)initWithFrame: (NSRect)frame andCanvas: (VALUE)c
{
  if ((self = [super initWithFrame: frame]))
  {
    canvas = c;
  }
  return self;
}
- (BOOL)isFlipped
{
  return YES;
}
- (void)drawRect: (NSRect)rect
{
  shoes_canvas *c;
  NSRect bounds = [self bounds];
  Data_Get_Struct(canvas, shoes_canvas, c);

  c->width = ROUND(bounds.size.width);
  c->height = ROUND(bounds.size.height);
  if (c->slot->vscroll)
  {
    [c->slot->vscroll setFrame: NSMakeRect(c->width - [NSScroller scrollerWidth], 0,
      [NSScroller scrollerWidth], c->height)];
    shoes_native_slot_lengthen(c->slot, c->height, c->endy);
  }
  c->place.iw = c->place.w = c->width;
  c->place.ih = c->place.h = c->height;
  c->slot->context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
  shoes_canvas_paint(canvas);
}
- (void)scroll: (NSScroller *)scroller
{
  shoes_canvas *c;
  Data_Get_Struct(canvas, shoes_canvas, c);

  switch ([scroller hitPart])
  {
    case NSScrollerIncrementLine:
      shoes_slot_scroll_to(c, 16, 1);
    break;
    case NSScrollerDecrementLine:
      shoes_slot_scroll_to(c, -16, 1);
    break;
    case NSScrollerIncrementPage:
      shoes_slot_scroll_to(c, c->height - 32, 1);
    break;
    case NSScrollerDecrementPage:
      shoes_slot_scroll_to(c, -(c->height - 32), 1);
    break;
    case NSScrollerKnobSlot:
    case NSScrollerKnob:
    default:
      shoes_slot_scroll_to(c, (c->endy - c->height) * [scroller floatValue], 0);
    break;
  }
}
@end


#if 0
@implementation ShoesTextView
- (id)initWithFrame: (NSRect)frame andObject: (VALUE)o
{
  if ((self = [super initWithFrame: frame]))
  {
    object = o;
    textView = [[NSTextView alloc] initWithFrame:
      NSMakeRect(0, 0, frame.size.width, frame.size.height)];
    [textView setVerticallyResizable: YES];
    [textView setHorizontallyResizable: YES];

    [self setBorderType: NSBezelBorder];
    [self setHasVerticalScroller: YES];
    [self setHasHorizontalScroller: NO];
    [self setDocumentView: textView];
    [textView setDelegate: (id<NSTextViewDelegate>)self];
  }
  return self;
}

// cjc - bug230 2014-07-26 - just had to learn that cocoa setEnabled is
// called for Shoes widget.state = or widget :state = "disabled"

-(void)setEnabled: (BOOL)enableIt
{
	//printf("setState called %d\n", enableIt);
  [textView setSelectable: enableIt];
  [textView setEditable: enableIt];
  if (enableIt)
    [textView setTextColor: [NSColor controlTextColor]];
  else
    [textView setTextColor: [NSColor disabledControlTextColor]];
}

-(NSTextStorage *)textStorage
{
  return [textView textStorage];
}
-(NSTextView *) textView
{
  return textView;
}
-(void)textDidChange: (NSNotification *)n
{
  shoes_control_send(object, s_change);
}
@end

// new for 3.2.25 - Subclass of ShoesTextView

@implementation ShoesTextEditView
- (id)initWithFrame: (NSRect)frame andObject: (VALUE)o
{
  printf( "cocoa: creating text_edit_view_frame\n");
  if ((self = [super initWithFrame: frame]))
  {
    printf("cocoa: creating text_edit_frame\n");
    object = o;
    textView = [[NSTextView alloc] initWithFrame:
      NSMakeRect(0, 0, frame.size.width, frame.size.height)];
    [textView setVerticallyResizable: YES];
    [textView setHorizontallyResizable: YES];

    [self setBorderType: NSBezelBorder];
    [self setHasVerticalScroller: YES];
    [self setHasHorizontalScroller: NO];
    [self setDocumentView: textView];
    [textView setDelegate: (id<NSTextViewDelegate>)self];
  }
  return self;
}

-(void)setEnabled: (BOOL)enableIt
{
	//printf("setState called %d\n", enableIt);
  [textView setSelectable: enableIt];
  [textView setEditable: enableIt];
  if (enableIt)
    [textView setTextColor: [NSColor controlTextColor]];
  else
    [textView setTextColor: [NSColor disabledControlTextColor]];
}

-(NSTextStorage *)textStorage
{
  return [textView textStorage];
}
-(void)textDidChange: (NSNotification *)n
{
  shoes_control_send(object, s_change);
}
@end
#endif 

@implementation ShoesNotifyDelegate 
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:self];
}

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification{
    return YES;
}
@end

#if 0
void
add_to_menubar(NSMenu *main, NSMenu *menu)
{
    NSMenuItem *dummyItem = [[NSMenuItem alloc] initWithTitle:@""
        action:nil keyEquivalent:@""];
    [dummyItem setSubmenu:menu];
    [main addItem:dummyItem];
    [dummyItem release];
}

void
create_apple_menu(NSMenu *main)
{
    NSMenuItem *menuitem;
    // Create the application (Apple) menu.
    NSMenu *menuApp = [[NSMenu alloc] initWithTitle: @"Apple Menu"];

    NSMenu *menuServices = [[NSMenu alloc] initWithTitle: @"Services"];
    [NSApp setServicesMenu:menuServices];

    menuitem = [menuApp addItemWithTitle:@"Open..."
        action:@selector(openFile:) keyEquivalent:@"o"];
    [menuitem setTarget: shoes_world->os.events];
#if 0    
    menuitem = [menuApp addItemWithTitle:@"Package..."
        action:@selector(package:) keyEquivalent:@"P"];
    [menuitem setTarget: shoes_world->os.events];
#endif    
    [menuApp addItemWithTitle:@"Preferences..." action:nil keyEquivalent:@""];
    [menuApp addItem: [NSMenuItem separatorItem]];
    menuitem = [[NSMenuItem alloc] initWithTitle: @"Services"
        action:nil keyEquivalent:@""];
    [menuitem setSubmenu:menuServices];
    [menuApp addItem: menuitem];
    [menuitem release];
    [menuApp addItem: [NSMenuItem separatorItem]];
    menuitem = [[NSMenuItem alloc] initWithTitle:@"Hide"
        action:@selector(hide:) keyEquivalent:@""];
    [menuitem setTarget: NSApp];
    [menuApp addItem: menuitem];
    [menuitem release];
    menuitem = [[NSMenuItem alloc] initWithTitle:@"Hide Others"
        action:@selector(hideOtherApplications:) keyEquivalent:@""];
    [menuitem setTarget: NSApp];
    [menuApp addItem: menuitem];
    [menuitem release];
    menuitem = [[NSMenuItem alloc] initWithTitle:@"Show All"
        action:@selector(unhideAllApplications:) keyEquivalent:@""];
    [menuitem setTarget: NSApp];
    [menuApp addItem: menuitem];
    [menuitem release];
    [menuApp addItem: [NSMenuItem separatorItem]];
    menuitem = [[NSMenuItem alloc] initWithTitle:@"Quit"
        action:@selector(terminate:) keyEquivalent:@"q"];
    [menuitem setTarget: NSApp];
    [menuApp addItem: menuitem];
    [menuitem release];
    // Turn off a warning message: laugh or cry?
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-method-access"
    // TODO: This undocumented method is required if you don't 
    // load a nib file which ties us up into xcode. Lesser to two evils.
    [NSApp setAppleMenu: menuApp]; 
#pragma clang diagnostic pop
    add_to_menubar(main, menuApp);
    [menuApp release];
}
#endif

void
create_edit_menu(NSMenu *main)
{
    NSMenuItem *menuitem;
    NSMenu *menuEdit = [[NSMenu alloc] initWithTitle: @"Edit"];

    menuitem = [menuEdit addItemWithTitle:@"Undo"
        action:@selector(undo:) keyEquivalent:@"z"];
    [menuitem setTarget: shoes_world->os.events];
    menuitem = [menuEdit addItemWithTitle:@"Redo"
        action:@selector(redo:) keyEquivalent:@"Z"];
    [menuitem setTarget: shoes_world->os.events];
    [menuEdit addItem: [NSMenuItem separatorItem]];
    menuitem = [menuEdit addItemWithTitle:@"Cut"
        action:@selector(cut:) keyEquivalent:@"x"];
    [menuitem setTarget: shoes_world->os.events];
    menuitem = [menuEdit addItemWithTitle:@"Copy"
        action:@selector(copy:) keyEquivalent:@"c"];
    [menuitem setTarget: shoes_world->os.events];
    menuitem = [menuEdit addItemWithTitle:@"Paste"
        action:@selector(paste:) keyEquivalent:@"v"];
    [menuitem setTarget: shoes_world->os.events];
    menuitem = [menuEdit addItemWithTitle:@"Select All"
        action:@selector(selectAll:) keyEquivalent:@"a"];
    [menuitem setTarget: shoes_world->os.events];
    add_to_menubar(main, menuEdit);
    [menuEdit release];
}

void
create_window_menu(NSMenu *main)
{
    NSMenu *menuWindows = [[NSMenu alloc] initWithTitle: @"Window"];

    [menuWindows addItemWithTitle:@"Minimize"
        action:@selector(performMiniaturize:) keyEquivalent:@""];
    [menuWindows addItemWithTitle:@"Close current Window"
        action:@selector(performClose:) keyEquivalent:@"w"];
    [menuWindows addItem: [NSMenuItem separatorItem]];
    [menuWindows addItemWithTitle:@"Bring All to Front"
        action:@selector(arrangeInFront:) keyEquivalent:@""];

    [NSApp setWindowsMenu:menuWindows];
    add_to_menubar(main, menuWindows);
    [menuWindows release];
}

void
create_help_menu(NSMenu *main)
{
    NSMenuItem *menuitem;
    NSMenu *menuHelp = [[NSMenu alloc] initWithTitle: @"Help"];
    menuitem = [menuHelp addItemWithTitle:@"Console"
        action:@selector(showLog:) keyEquivalent:@"/"];
    [menuitem setTarget: shoes_world->os.events];
    [menuHelp addItem: [NSMenuItem separatorItem]];
    menuitem = [menuHelp addItemWithTitle:@"Manual"
        action:@selector(help:) keyEquivalent:@"m"];
    [menuitem setTarget: shoes_world->os.events];
    add_to_menubar(main, menuHelp);
    [menuHelp release];
}

VALUE
shoes_font_list()
{
  INIT;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
  VALUE ary = rb_ary_new();
  // CFBooleanRef value= kCFBooleanTrue;
  int vtrue[1] = {1};
  CFDictionaryRef dict = CFDictionaryCreate(NULL,
					    (const void **)kCTFontCollectionRemoveDuplicatesOption,
					    (const void **)&vtrue, 1, NULL, NULL);
  CTFontCollectionRef fcref = CTFontCollectionCreateFromAvailableFonts(dict);
  CFArrayRef arrayref = CTFontCollectionCreateMatchingFontDescriptors(fcref);
  CFRelease(fcref);
  CFIndex count = CFArrayGetCount(arrayref);
  CFIndex i;
  for (i=0; i<count; i++) {
    CTFontDescriptorRef fdesc = (CTFontDescriptorRef)CFArrayGetValueAtIndex(arrayref, i);
    CTFontRef font = CTFontCreateWithFontDescriptor(fdesc, 0., NULL);
    CFStringRef cfname = CTFontCopyFullName(font);
    char fname[256];  // Use local buffer instead of static
    if (CFStringGetCString(cfname, fname, sizeof(fname), kCFStringEncodingUTF8)) {
      rb_ary_push(ary, rb_str_new2(fname));
    }
    CFRelease(cfname);  // Release the copied string
    CFRelease(font);    // Release the created font
  }
  CFRelease(arrayref);  // Release the array
  CFRelease(dict);      // Release the dictionary
#else
  ATSFontIterator fi = NULL;
  ATSFontRef fontRef = 0;
  NSMutableArray *outArray;
  VALUE ary = rb_ary_new();
  if (noErr == ATSFontIteratorCreate(kATSFontContextLocal, nil, nil,
         kATSOptionFlagsUnRestrictedScope, &fi))
  {
    while (noErr == ATSFontIteratorNext(fi, &fontRef))
    {
      NSString *fontName;
      ATSFontGetName(fontRef, kATSOptionFlagsDefault, &fontName);
      if (fontName != NULL)
        rb_ary_push(ary, rb_str_new2([fontName UTF8String]));
    }
  }

  ATSFontIteratorRelease(&fi);
#endif
  RELEASE;
  rb_funcall(ary, rb_intern("uniq!"), 0);
  rb_funcall(ary, rb_intern("sort!"), 0);
  return ary;
}

VALUE
shoes_load_font(const char *filename)
{
  VALUE families = Qnil;
#ifndef OLD_OSX
  CFURLRef cfuref ;
  bool ok;
  CFErrorRef err;
  cfuref = CFURLCreateFromFileSystemRepresentation (NULL, (UInt8 *)filename, strlen(filename), false);
  // Get array of CTFontDescriptorRef's
  CFArrayRef arrayref = CTFontManagerCreateFontDescriptorsFromURL(cfuref);
  CFIndex count = CFArrayGetCount(arrayref);
  CFIndex i;
  if (count == 0)
    return Qnil;
  families = rb_ary_new();
  // get the names 
  for (i=0; i<count; i++) {
    CTFontDescriptorRef fdesc =(CTFontDescriptorRef)CFArrayGetValueAtIndex(arrayref, i);
    CTFontRef font = CTFontCreateWithFontDescriptor(fdesc, 0., NULL);
	CFStringRef cfname = CTFontCopyFullName(font);
	static char fname[100];
	CFStringGetCString(cfname, fname, sizeof(fname), kCFStringEncodingUTF8);
    rb_ary_push(families, rb_str_new2(fname));
  }
  // Register the font
  ok = CTFontManagerRegisterFontsForURL(cfuref, kCTFontManagerScopeProcess, &err);
  if (!ok ) {
    NSLog(@"Failed CTFontManager");
  } 
#else
  FSRef fsRef;
  FSSpec fsSpec;
  Boolean isDir;
  ATSFontContainerRef ref;
  NSString *fontName;
  FSPathMakeRef(filename, &fsRef, &isDir);
  if (FSGetCatalogInfo(&fsRef, kFSCatInfoNone, NULL, NULL, &fsSpec, NULL) == noErr)
  {
    ATSFontActivateFromFileReference(&fsRef, kATSFontContextLocal, kATSFontFormatUnspecified,
      NULL, kATSOptionFlagsDefault, &ref);
    if (ref != NULL)
    {
      int i = 0;
      ItemCount count = 0;
      ATSFontRef *fonts;
      ATSFontFindFromContainer(ref, kATSOptionFlagsDefault, 0, NULL, &count);
      families = rb_ary_new();
      if (count > 0)
      {
        fonts = SHOE_ALLOC_N(ATSFontRef, count);
        ATSFontFindFromContainer(ref, kATSOptionFlagsDefault, count, fonts, &count);
        for (i = 0; i < count; i++)
        {
          fontName = NULL;
          ATSFontGetName(fonts[i], kATSOptionFlagsDefault, &fontName);
          if (fontName != NULL)
            rb_ary_push(families, rb_str_new2([fontName UTF8String]));
        }
        SHOE_FREE(fonts);
      }
    }
  }
#endif
  shoes_update_fonts(shoes_font_list());
  return families;
}

void shoes_native_init()
{
  fprintf(stderr, "[COCOA] shoes_native_init: Starting\n");
  fprintf(stderr, "[COCOA] shoes_native_init: shoes_world=%p\n", (void*)shoes_world);
  
  if (!shoes_world) {
    fprintf(stderr, "[COCOA] shoes_native_init: ERROR - shoes_world is NULL!\n");
    return;
  }
  
  INIT;
  NSTimer *idle;
  NSApplication *NSApp = [NSApplication sharedApplication];
  NSMenu *main = [[NSMenu alloc] initWithTitle: @""];
  
  fprintf(stderr, "[COCOA] shoes_native_init: Creating ShoesEvents\n");
  shoes_world->os.events = [[ShoesEvents alloc] init];
  fprintf(stderr, "[COCOA] shoes_native_init: shoes_world->os.events=%p\n", (void*)shoes_world->os.events);
  [NSApp setMainMenu: main];
  shoes_native_menu_root(main);
#if 0
  create_apple_menu(main);
  create_edit_menu(main);
  create_window_menu(main);
  create_help_menu(main);
#endif 
  [NSApp setDelegate: (id<NSApplicationDelegate>)shoes_world->os.events];

  idle = [NSTimer scheduledTimerWithTimeInterval: 0.01
    target: shoes_world->os.events selector: @selector(idle:) userInfo: nil
    repeats: YES];
  [[NSRunLoop currentRunLoop] addTimer: idle forMode: NSEventTrackingRunLoopMode];
  // 2nd stage of stdout setup. Is this the right spot in time?
  shoes_osx_stdout_sink();
  RELEASE;
}

void shoes_native_cleanup(shoes_world_t *world)
{
  INIT;
  [shoes_world->os.events release];
  RELEASE;
}

void shoes_native_quit()
{
  INIT;
  NSApplication *NSApp = [NSApplication sharedApplication];
  [NSApp stop: nil];
  RELEASE;
}

void shoes_get_time(SHOES_TIME *ts)
{
  gettimeofday(ts, NULL);
}

unsigned long shoes_diff_time(SHOES_TIME *start, SHOES_TIME *end)
{
  unsigned long usec;
  if ((end->tv_usec-start->tv_usec)<0) {
    usec = (end->tv_sec-start->tv_sec - 1) * 1000;
    usec += (1000000 + end->tv_usec - start->tv_usec) / 1000;
  } else {
    usec = (end->tv_sec - start->tv_sec) * 1000;
    usec += (end->tv_usec - start->tv_usec) / 1000;
  }
  return usec;
}

int shoes_throw_message(unsigned int name, VALUE obj, void *data)
{
  return shoes_catch_message(name, obj, data);
}

void shoes_native_slot_mark(SHOES_SLOT_OS *slot)
{
  rb_gc_mark_maybe(slot->controls);
}

void shoes_native_slot_reset(SHOES_SLOT_OS *slot)
{
  slot->controls = rb_ary_new();
  rb_gc_register_address(&slot->controls);
}

void shoes_native_slot_clear(shoes_canvas *canvas)
{
  rb_ary_clear(canvas->slot->controls);
  if (canvas->slot->vscroll)
  {
    shoes_native_slot_lengthen(canvas->slot, canvas->height, 1);
  }
}

void shoes_native_slot_paint(SHOES_SLOT_OS *slot)
{
  [slot->view setNeedsDisplay: YES];
}

void shoes_native_slot_lengthen(SHOES_SLOT_OS *slot, int height, int endy)
{
  if (slot->vscroll)
  {
    double s = slot->scrolly * 1., e = endy * 1., h = height * 1., d = (endy - height) * 1.;
    COCOA_DO({
      [slot->vscroll setDoubleValue: (d > 0 ? s / d : 0)];
      [slot->vscroll setKnobProportion: (h / e)];
      [slot->vscroll setHidden: endy <= height ? YES : NO];
    });
  }
}

void shoes_native_slot_scroll_top(SHOES_SLOT_OS *slot)
{
}

int shoes_native_slot_gutter(SHOES_SLOT_OS *slot)
{
  return (int)[NSScroller scrollerWidth];
}

void shoes_native_remove_item(SHOES_SLOT_OS *slot, VALUE item, char c)
{
  if (c)
  {
    long i = rb_ary_index_of(slot->controls, item);
    if (i >= 0)
      rb_ary_insert_at(slot->controls, i, 1, Qnil);
  }
}

shoes_code
shoes_app_cursor(shoes_app *app, ID cursor)
{
  if (app->os.window == NULL || app->cursor == cursor)
    goto done;

  if (cursor == s_hand_cursor || cursor == s_link)
    [[NSCursor pointingHandCursor] set];
  else if (cursor == s_arrow_cursor)
    [[NSCursor arrowCursor] set];
  else if (cursor == s_text_cursor)
    [[NSCursor IBeamCursor] set];
  else if (cursor == s_watch_cursor)
    [[NSCursor operationNotAllowedCursor] set];
  else
    goto done;

  app->cursor = cursor;

done:
  return SHOES_OK;
}


//  ---- window resize ----
void
shoes_native_app_resize_window(shoes_app *app)
{
  NSRect rect = [app->os.window frame];
  rect.size.width = app->width;
  rect.size.height = app->height;
  [app->os.window setFrame: rect display: YES];
}

VALUE shoes_native_get_resizable(shoes_app *app) 
{
  NSWindow *win = app->os.window;
  if ([win styleMask] == NSResizableWindowMask)
     return Qtrue;
  else 
    return Qfalse;
} 

void shoes_native_set_resizable(shoes_app *app, int resizable)
{
  NSWindow *win = app->os.window;
  if (resizable)
    [win setStyleMask:[win styleMask] | NSResizableWindowMask];
  else
    [win setStyleMask:[win styleMask] & ~NSResizableWindowMask];
}


void
shoes_native_app_title(shoes_app *app, char *msg)
{
  COCOA_DO([app->os.window setTitle: [NSString stringWithUTF8String: msg]]);
}

// new with 3.2.19
void
shoes_native_app_set_wtitle(shoes_app *app, char *wtitle)
{
  COCOA_DO([app->os.window setTitle: [NSString stringWithUTF8String: wtitle]]);
}

// new with 3.2.19
void
shoes_native_app_set_icon(shoes_app *app, char *icon_path)
{
	NSImage *icon = [[NSImage alloc] initWithContentsOfFile: [NSString stringWithUTF8String: icon_path]];
  [NSApp setApplicationIconImage: icon];
}

// Helper app->monitor to screen
NSScreen * shoes_native_app_screen(shoes_app *app) {
  int cnt = 0, realmon;
  NSArray *screens = [NSScreen screens];
  cnt = [screens count];
  if (app->monitor < cnt && app->monitor >= 0)
    realmon = app->monitor;
  else {
    return [NSScreen mainScreen];
  }
  return screens[realmon];
}

/*
 * We don't really care about which CGdevice is mainScreen 
 * We ASSUME device table to be in sync with the NSScreen array.
 */
 
CGDirectDisplayID shoes_native_app_cgid(shoes_app *app) {
  CGDirectDisplayID devids[4];
  int i, cnt;
  if (app->monitor < 0)
    return CGMainDisplayID();
    
  // get all the display devices (upto 4);  
  CGGetActiveDisplayList(4, devids, &cnt);
  return devids[app->monitor];
}

static ShoesWindow *
shoes_native_app_window(shoes_app *app, int dialog)
{
  ShoesWindow *window;
  unsigned int mask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
  NSRect rect = NSMakeRect(0, 0, app->width, app->height);
  NSSize size = {app->minwidth, app->minheight};
  // 3.2.24 constrain to screen size-docsize-menubar (visibleFrame)
  // Then minus the window's title bar
  NSRect screenr;
  NSScreen *screen;
  screen = shoes_native_app_screen(app);
  screenr = [screen visibleFrame];

  if (app->height > screenr.size.height) {
    app->height = screenr.size.height;
    rect.size.height = screenr.size.height;
    app->height = app->height - 20;
    rect.size.height = rect.size.height - 20;
  }
  if (app->resizable)
    mask |= NSResizableWindowMask;
    
  if (app->fullscreen) {
    mask = (NSBorderlessWindowMask | NSFullScreenWindowMask);
    rect = [screen frame];
    // TODO:  setting rect to frame is not enough for fullscreen (y == 63 for me)
  }

  //window = [[ShoesWindow alloc] initWithContentRect: rect
  //  styleMask: mask backing: NSBackingStoreBuffered defer: NO];

  //NSLog(@"rect: %4.0f, %4.0f, %4.0f, %4.0f\n:",rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
  //NSLog(@"news: %4.0f, %4.0f, %4.0f, %4.0f\n:",screenr.origin.x, screenr.origin.y, screenr.size.width, screenr.size.height);
  //NSLog(@"npos: %4.0f, %4.0f, %4.0f, %4.0f\n:",rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
  
  window = [[ShoesWindow alloc] initWithContentRect: rect
    styleMask: mask backing: NSBackingStoreBuffered defer: NO screen: screen];

  if (app->minwidth > 0 || app->minheight > 0)
    [window setContentMinSize: size];
  // OSX BUG!! Center doesn't work on muliple monitors
  if (app->monitor == -1) {
    [window center];
  } else {
    if (! app->fullscreen) {
      // do our own center, reuse rect the variable.
      rect.origin.x = screenr.origin.x + ((screenr.size.width - rect.size.width) / 2.0);
      rect.origin.y = screenr.origin.y + ((screenr.size.height - rect.size.height) / 2.0);
    } 
    [window setFrame: rect display: YES];
  }  
  [window prepareWithApp: app->self];
  if (app->fullscreen) {
    /* can't use shoes_native_app_fullscreen because it would call us recursively
     * so, we dup some code. Note: we have to be able to un-fullscreene
     * from `app.fullscreen = false` in a script
    */
    int level;
    CGDirectDisplayID devid = shoes_native_app_cgid(app);
    if (CGDisplayCapture(devid) != kCGErrorSuccess)
      return window; 		// TODO: Not the best notirication method
    app->os.monitor = devid;
    app->os.normal = [screen visibleFrame];
    level = CGShieldingWindowLevel();
    app->width = ROUND(screenr.size.width);
    app->height = ROUND(screenr.size.height);
    app->os.window = window;
    [window setLevel: level];
    app->os.view = [window contentView];
  }
  return window;
}

void
shoes_native_view_supplant(NSView *from, NSView *to)
{
  for (id subview in [from subviews])
    [to addSubview:subview];
}


 
void
shoes_native_app_fullscreen(shoes_app *app, char yn)
{
  ShoesWindow *old = app->os.window;

  if (yn)
  {
    int level;
    NSRect screenr;
    NSScreen *screen = shoes_native_app_screen(app); 
    // NSNumber *didn = old.deviceDescription[@"NSScreenNumber" ]; // sigh, doesn't work!
    CGDirectDisplayID devid = shoes_native_app_cgid(app);
    if (CGDisplayCapture(devid) != kCGErrorSuccess)
      return;
    app->os.monitor = devid;
    app->os.normal = [old frame];
    level = CGShieldingWindowLevel();
    screenr = [screen frame];
    COCOA_DO({
      app->width = ROUND(screenr.size.width);
      app->height = ROUND(screenr.size.height);
      app->os.window = shoes_native_app_window(app, 0);
      [app->os.window setLevel: level];
      shoes_native_view_supplant([old contentView], [app->os.window contentView]);
      app->os.view = [app->os.window contentView];
      [old disconnectApp];
      [old close];
      [app->os.window setFrame: screenr display: YES];
    });
  }
  else
  {
    COCOA_DO({
      app->width = ROUND(app->os.normal.size.width);
      app->height = ROUND(app->os.normal.size.height);
      app->os.window = shoes_native_app_window(app, 0);
      [app->os.window setLevel: NSNormalWindowLevel];
      CGDisplayRelease(app->os.monitor);
      shoes_native_view_supplant([old contentView], [app->os.window contentView]);
      app->os.view = [app->os.window contentView];
      [old disconnectApp];
      [old close];
      [app->os.window setFrame: app->os.normal display: YES];
    });
  }
}

shoes_code
shoes_native_app_open(shoes_app *app, char *path, int dialog, shoes_settings *st)
{
  shoes_code code = SHOES_OK;
  app->os.normal = NSMakeRect(0, 0, app->width, app->height);
  COCOA_DO({
    app->os.window = shoes_native_app_window(app, dialog);
    app->slot->view = [app->os.window contentView];
  });
//quit:
  return code;
}

void
shoes_native_app_show(shoes_app *app)
{
  COCOA_DO([app->os.window orderFront: nil]);
}

void
shoes_native_loop()
{
  NSApplication *NSApp = [NSApplication sharedApplication];
  [NSApp run];
}

void
shoes_native_app_close(shoes_app *app)
{
  COCOA_DO([app->os.window close]);
}

void
shoes_browser_open(char *url)
{
  VALUE browser = rb_str_new2("open ");
  rb_str_cat2(browser, url);
  shoes_sys(RSTRING_PTR(browser), 1);
}

void
shoes_slot_init(VALUE c, SHOES_SLOT_OS *parent, int x, int y, int width, int height, int scrolls, int toplevel)
{
  shoes_canvas *canvas;
  SHOES_SLOT_OS *slot;
  Data_Get_Struct(c, shoes_canvas, canvas);

  COCOA_DO({
    slot = shoes_slot_alloc(canvas, parent, toplevel);
    slot->controls = parent->controls;
    slot->view = [[ShoesView alloc] initWithFrame: NSMakeRect(x, y, width, height) andCanvas: c];
    [slot->view setAutoresizesSubviews: NO];
    if (toplevel)
      [slot->view setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];
    slot->vscroll = NULL;
    if (scrolls)
    {
      slot->vscroll = [[NSScroller alloc] initWithFrame:
        NSMakeRect(width - [NSScroller scrollerWidth], 0, [NSScroller scrollerWidth], height)];
      [slot->vscroll setEnabled: YES];
      [slot->vscroll setTarget: slot->view];
      [slot->vscroll setAction: @selector(scroll:)];
      [slot->view addSubview: slot->vscroll];
    }
    if (parent->vscroll)
      [parent->view addSubview: slot->view positioned: NSWindowBelow relativeTo: parent->vscroll];
    else
      [parent->view addSubview: slot->view];
  });
}

void
shoes_slot_destroy(shoes_canvas *canvas, shoes_canvas *pc)
{
  INIT;
  if (canvas->slot->vscroll != NULL)
    [canvas->slot->vscroll removeFromSuperview];
  [canvas->slot->view removeFromSuperview];
  RELEASE;
}

cairo_t *
shoes_cairo_create(shoes_canvas *canvas)
{
  cairo_t *cr;
  canvas->slot->surface = cairo_quartz_surface_create_for_cg_context(canvas->slot->context,
    canvas->width, canvas->height);
  cr = cairo_create(canvas->slot->surface);
  cairo_translate(cr, 0, 0 - canvas->slot->scrolly);
  return cr;
}

void shoes_cairo_destroy(shoes_canvas *canvas)
{
  cairo_surface_destroy(canvas->slot->surface);
}

void
shoes_group_clear(SHOES_GROUP_OS *group)
{
}

void
shoes_native_canvas_place(shoes_canvas *self_t, shoes_canvas *pc)
{
  NSRect rect, rect2;
  int newy = (self_t->place.iy + self_t->place.dy) - pc->slot->scrolly;
  rect.origin.x = (self_t->place.ix + self_t->place.dx) * 1.;
  rect.origin.y = ((newy) * 1.);
  rect.size.width = (self_t->place.iw * 1.);
  rect.size.height = (self_t->place.ih * 1.);
  rect2 = [self_t->slot->view frame];
  if (rect.origin.x != rect2.origin.x || rect.origin.y != rect2.origin.y ||
      rect.size.width != rect2.size.width || rect.size.height != rect2.size.height)
  {
    [self_t->slot->view setFrame: rect];
  }
}

void
shoes_native_canvas_resize(shoes_canvas *canvas)
{
  NSSize size = {canvas->width, canvas->height};
  [canvas->slot->view setFrameSize: size];
}


void
shoes_native_control_hide(SHOES_CONTROL_REF ref)
{
  COCOA_DO([ref setHidden: YES]);
}

void
shoes_native_control_show(SHOES_CONTROL_REF ref)
{
  COCOA_DO([ref setHidden: NO]);
}

static void
shoes_native_control_frame(SHOES_CONTROL_REF ref, shoes_place *p)
{
  NSRect rect;
  rect.origin.x = p->ix + p->dx;
  rect.origin.y = p->iy + p->dy;
  rect.size.width = p->iw;
  rect.size.height = p->ih;
  [ref setFrame: rect];
}

void
shoes_native_control_position(SHOES_CONTROL_REF ref, shoes_place *p1, VALUE self,
  shoes_canvas *canvas, shoes_place *p2)
{
  PLACE_COORDS();
  if (canvas->slot->vscroll)
    [canvas->slot->view addSubview: ref positioned: NSWindowBelow relativeTo: canvas->slot->vscroll];
  else
    [canvas->slot->view addSubview: ref];
  shoes_native_control_frame(ref, p2);
  rb_ary_push(canvas->slot->controls, self);
}

void
shoes_native_control_repaint(SHOES_CONTROL_REF ref, shoes_place *p1,
  shoes_canvas *canvas, shoes_place *p2)
{
  p2->iy -= canvas->slot->scrolly;
  if (CHANGED_COORDS()) {
    PLACE_COORDS();
    shoes_native_control_frame(ref, p2);
  }
  p2->iy += canvas->slot->scrolly;
}

/*  
 * In Cocoa, control refs can be different cocoa classes so
 * buttons don't have the same methods and properties as say an edit_box
 * buttons are NSControl
 * COCOA_DO macro can't be stepped through with lldb and we shouldn't depend
 * on try/release to help us for this function.
 */
void
shoes_native_control_state(SHOES_CONTROL_REF ref, BOOL sensitive, BOOL setting)
{
  if ([ref isKindOfClass: [NSControl class]]) {
    [ref setEnabled: sensitive];
    if ([ref respondsToSelector: @selector(setEditable:)]) {
      [(NSTextField *)ref setEditable: setting];
    }
  } else if ([ref isKindOfClass: [NSScrollView class]]) {
      ShoesTextView *sv = (ShoesTextView *)ref;
      NSTextView *tv = sv->textView;
      [tv setEditable: setting];
  } else if ([ref isKindOfClass: [NSProgressIndicator class]]) {
      // not really a control
  } else {
    fprintf(stderr, "control is unknown type\n");
  }
}

void
shoes_native_control_focus(SHOES_CONTROL_REF ref)
{
  COCOA_DO([[ref window] makeFirstResponder: ref]);
}

void
shoes_native_control_remove(SHOES_CONTROL_REF ref, shoes_canvas *canvas)
{
  COCOA_DO([ref removeFromSuperview]);
}

void
shoes_native_control_free(SHOES_CONTROL_REF ref)
{
}


#if 0
// text_edit_box is new in 3.2.25
void
shoes_native_text_view_set_text(SHOES_CONTROL_REF ref, char *msg)
{
  COCOA_DO([[[(ShoesTextEditView *)ref textStorage] mutableString] setString: [NSString stringWithUTF8String: msg]]);
}

SHOES_CONTROL_REF
shoes_native_text_view(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  INIT;
  ShoesTextEditView *tv = [[ShoesTextEditView alloc] initWithFrame:
    NSMakeRect(place->ix + place->dx, place->iy + place->dy,
    place->ix + place->dx + place->iw, place->iy + place->dy + place->ih)
    andObject: self];
  //shoes_native_text_view_set_text((NSControl *)tv, msg);
  shoes_native_text_view_set_text((SHOES_CONTROL_REF)tv, msg);
  RELEASE;
  return (NSControl *)tv;
}

VALUE
shoes_native_text_view_get_text(SHOES_CONTROL_REF ref)
{
  VALUE text = Qnil;
  INIT;
  text = rb_str_new2([[[(ShoesTextView *)ref textStorage] string] UTF8String]);
  RELEASE;
  return text;
}


VALUE
shoes_native_text_view_append(SHOES_CONTROL_REF ref, char *msg)
{
  COCOA_DO([[[(ShoesTextEditView *)ref textStorage] mutableString] appendString: [NSString stringWithUTF8String: msg]]);
#ifdef dontwant
  // do not like
  NSAttributedString *atext;
  NSTextStorage *buffer;
  INIT;
  NSString *utext = [[NSString alloc] initWithCString: msg encoding: NSUTF8StringEncoding];
  atext = [[NSAttributedString alloc] initWithString: utext];
  buffer = [[(ShoesTextEditView *)ref textStorage] mutableString];
  //  [[self textStorage] appendAttributedString: attrStr];
  [buffer appendAttributedString: atext];
  //[self scrollRangeToVisible:NSMakeRange([[self string] length], 0)];
  RELEASE;
#endif
  return Qnil;
}
#endif


VALUE
shoes_native_clipboard_get(shoes_app *app)
{
  VALUE txt = Qnil;
  INIT;
  NSString *paste = [[NSPasteboard generalPasteboard] stringForType: NSStringPboardType];
  if (paste) txt = rb_str_new2([paste UTF8String]);
  RELEASE;
  return txt;
}

void
shoes_native_clipboard_set(shoes_app *app, VALUE string)
{
  COCOA_DO({
    [[NSPasteboard generalPasteboard] declareTypes: [NSArray arrayWithObject: NSStringPboardType] owner: nil];
    [[NSPasteboard generalPasteboard] setString: [NSString stringWithUTF8String: RSTRING_PTR(string)]
      forType: NSStringPboardType];
  });
}

VALUE
shoes_native_to_s(VALUE text)
{
  text = rb_funcall(text, s_to_s, 0);
  return text;
}

VALUE
shoes_native_window_color(shoes_app *app)
{
  CGFloat r, g, b, a;
  INIT;
  [[[app->os.window backgroundColor] colorUsingColorSpace: [NSColorSpace genericRGBColorSpace]]
     getRed: &r green: &g blue: &b alpha: &a];
  RELEASE;
  //return shoes_color_new((int)(r * 255), (int)(g * 255), (int)(b * 255), (int)(a * 255));
  return shoes_color_new(255, 255, 255, 255);
}


void shoes_native_systray(char *title, char *message, char *path)
{
    NSUserNotification *notification = [[NSUserNotification alloc] init];
    notification.title = [NSString stringWithUTF8String: title];
    notification.informativeText = [NSString stringWithUTF8String: message];
    notification.soundName = NSUserNotificationDefaultSoundName;
    notification.contentImage = [[NSImage alloc] initWithContentsOfFile: [NSString stringWithUTF8String: path]];
    [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:notification];
}


/*
 * ---- tooltips ----
 * TODO: needs attr handling in each of the widgets. 
 * are a property of NSView - damn near everything in osx but we might
 * want to make sure it's a SHOES_CONTROL_REF/shoes-widget.
*/
void shoes_native_control_set_tooltip(SHOES_CONTROL_REF ref, VALUE tooltip)
{ 
    NSView *view = (NSView *)ref;
    view.toolTip = [NSString stringWithUTF8String: RSTRING_PTR(tooltip)];
    
}

VALUE shoes_native_control_get_tooltip(SHOES_CONTROL_REF ref) {
  NSView *view = (NSView *)ref;
  return rb_str_new2((char *)view.toolTip);
}

// ---- opacity ----
double shoes_native_app_get_opacity(shoes_app *app) 
{
  NSWindow *win = (NSWindow *)app->os.window;
  return [win alphaValue];
}

void shoes_native_app_set_opacity(shoes_app *app, double opacity)
{
  NSWindow *win = (NSWindow *)app->os.window;
  [win setAlphaValue: opacity];
}



// ---- decoration remove title bar, resize controls ----
void shoes_native_app_set_decoration(shoes_app *app, gboolean decorated)
{
}

int shoes_native_app_get_decoration(shoes_app *app)
{
  return true;
}

// ---- window positioning calls from Shoes ----

/* 
    OSX origin is 0,0  bottom left. Shoes is top,left
    Cocoa wont move past  menubar or completely below the dock.
    launcher - shoes reports otherwise. So we report where it was
    really was moved to.

    NOTE for multiple monitors we uss the large space.
*/
void shoes_native_app_get_window_position(shoes_app *app) {
  NSRect screenr = [shoes_native_app_screen(app) frame];
  NSWindow *win = (NSWindow *)app->os.window;
  NSRect frame = [win frame];
  //NSLog(@"current: x, y h: %i, %i %i", (int) frame.origin.x, (int) frame.origin.y, 
  //   (int) frame.size.height);
  app->x = frame.origin.x;
  app->y = screenr.size.height - frame.origin.y;
}


void shoes_native_app_window_move(shoes_app *app, int x, int y) {
  NSRect screenr = [shoes_native_app_screen(app) frame];
  //NSLog(@"screen h,w: %i, %i",(int) screen.size.height, (int)screen.size.width);
  NSWindow *win = (NSWindow *)app->os.window;
  NSRect frame = [win frame];
  frame.origin.x = x;
  // convert y from Shoes(t,l) to osx(b,l)
  frame.origin.y = screenr.size.height - y;
  [win setFrame: frame display: YES animate: NO];
  
  // get where the frame really moved to because OSX may contrain the move.
  frame = [win frame];
  app->x = frame.origin.x;
  app->y = screenr.size.height - frame.origin.y;
  //NSLog(@"real pos %i,%i return y: %i", (int)frame.origin.x, (int)frame.origin.y, app->y);
  
}

/* 
 * Helper function to return a NSDict for use in AttributedString creation
 * input is the Shoes attrs (font and stroke)
*/
NSMutableDictionary *shoes_attr_dict(VALUE attr) {
  char *fntstr = 0;
  VALUE fgclr = Qnil; // Could be hex color string or Shoes color object
  NSInteger fsize = 0;
  NSArray *fontsettings;
  NSMutableDictionary *dict = NULL;
  NSMutableString *fontname = [[NSMutableString alloc] initWithCapacity: 40];


  // get the Shoes attributes 
  if (!NIL_P(shoes_hash_get(attr, rb_intern("font")))) {
    fntstr = RSTRING_PTR(shoes_hash_get(attr, rb_intern("font")));
    NSString *fstr = [NSString stringWithUTF8String: fntstr];
    fontsettings = [fstr componentsSeparatedByString:@" "]; 
    // in OSX there is font name - may include Bold etc, and size
    int cnt = fontsettings.count;
    fsize = [fontsettings[cnt-1] integerValue];
    if (fsize > 0 && fsize < 24)  {
      //we probably have a size spec - everything before that is fontname
      int i;
      for (i = 0; i < cnt-1; i++) {
       [fontname appendString: fontsettings[i]];
       if (i < cnt-2) {
         [fontname appendString:@" "];
       }
      }
    } else {
      // have to assume they didn't give a point size so 
      [fontname  appendString: fstr];
      fsize = 10;
    }
  }
  if (!NIL_P(shoes_hash_get(attr, rb_intern("stroke")))) {
    fgclr = shoes_hash_get(attr, rb_intern("stroke"));
  }
  if (fntstr || !NIL_P(fgclr)) {
    dict = [[NSMutableDictionary alloc] initWithCapacity: 5];
    //NSString *title = [NSString stringWithUTF8String: msg];
    if (fntstr) {
      NSFont *font = [NSFont fontWithName: fontname size: fsize];
      if (font == nil) 
        // Don't do this : rb_raise(rb_eArgError, "Font \"%s\" not found", fntstr);
        font = [NSFont fontWithName: @"arial" size: 12];
      [dict setObject: font forKey: NSFontAttributeName];
    }
    if (! NIL_P(fgclr)) {
      // convert Shoes color to NSColor
      if (TYPE(fgclr) == T_STRING) 
        fgclr = shoes_color_parse(cColor, fgclr);  // convert string to cColor
      if (rb_obj_is_kind_of(fgclr, cColor)) 
      { 
        shoes_color *color; 
        Data_Get_Struct(fgclr, shoes_color, color); 
        CGFloat rg = (CGFloat)color->r / 255;
        CGFloat gb = (CGFloat)color->g / 255;
        CGFloat bb = (CGFloat)color->b / 255;
        NSColor *clr = [NSColor colorWithCalibratedRed: rg green: gb blue: bb alpha: 1.0];
        [dict setObject: clr forKey: NSForegroundColorAttributeName];
      }
    }
  }
  return dict;
}

// -------- monitor routines ------
int shoes_native_monitor_count() {
  NSArray *scns = [NSScreen screens];
  return [scns count];
}

int shoes_native_monitor_default() {
  NSScreen *main = [NSScreen mainScreen];
  int i;
  NSArray *screens = [NSScreen screens];
  for (i = 0; i < [screens count]; i++) {
    if (screens[i] == main)  return i;
  }
  // Should not happen
  return 0;
}

void shoes_native_monitor_geometry(int mon, shoes_monitor_t *r) {
  NSScreen *scn;
  NSArray *screens = [NSScreen screens];
  scn = screens[mon];
  NSRect rect = [scn visibleFrame];
  r->x = rect.origin.x;
  r->y = rect.origin.y;
  r->width = rect.size.width;
  r->height = rect.size.height;
}

void shoes_native_monitor_set(shoes_app *app) {

  NSScreen *ns = shoes_native_app_screen(app);
  NSRect nr = [ns visibleFrame];
  
  NSWindow *win = (NSWindow *)app->os.window;
  NSRect os = [[win screen] frame];
  NSRect or = [win frame];   // get current position
  //NSLog(@"os: %4.0f, %4.0f, %4.0f, %4.0f\n", os.origin.x, os.origin.y, os.size.width, os.size.height);
  //NSLog(@"or: %4.0f, %4.0f, %4.0f, %4.0f\n", or.origin.x, or.origin.y, or.size.width, or.size.height);
  //NSLog(@"nr: %4.0f, %4.0f, %4.0f, %4.0f\n", nr.origin.x, nr.origin.y, nr.size.width, nr.size.height);
  
  nr.origin.x += (or.origin.x - os.origin.x);
  nr.origin.y += (or.origin.y - os.origin.y);
  nr.size.width = app->width;
  nr.size.height = app->height;
  [win setFrame: nr display: YES];
}

int shoes_native_monitor_get(shoes_app *app) {
  NSScreen *screen = [app->os.window screen];
  int i;
  NSArray *screens = [NSScreen screens];
  for (i = 0; i < [screens count]; i++) {
    if (screens[i] == screen)
      return i;
  }
  // Should not happen
  return 0;
}
