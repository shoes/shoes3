//
// shoes/native/cocoa.h
// Custom Cocoa interfaces for Shoes
//
#import <Cocoa/Cocoa.h>
#ifndef OLD_OSX
#import <AppKit/NSFontCollection.h>
#endif

@interface ShoesEvents : NSObject
{
  int count;
}
@end

@interface ShoesWindow : NSWindow
{
  VALUE app;
}
@end

@interface ShoesView : NSView
{
  VALUE canvas;
}
@end

@interface ShoesTimer : NSObject
{
  VALUE object;
  NSTimer *timer;
}
@end

// Who needs this? shoes_canvas_send_start (canvas.c)
@interface CanvasOneShot : NSObject
{
  VALUE object;
  NSTimer *timer;
}
@end

@interface ShoesNotifyDelegate : NSObject <NSApplicationDelegate, NSUserNotificationCenterDelegate>
@end

// declares to stop compiler whining
void add_to_menubar(NSMenu *main, NSMenu *menu);
void create_apple_menu(NSMenu *main);
void create_edit_menu(NSMenu *main);
void create_window_menu(NSMenu *main);
void create_help_menu(NSMenu *main);
void shoes_native_view_supplant(NSView *from, NSView *to);
// void gettimeofday(void *ts, void *extra);  // Commented out - conflicts with system header
NSMutableDictionary *shoes_attr_dict(VALUE attr);

#define VK_ESCAPE 53
#define VK_DELETE 117
#define VK_INSERT 114
#define VK_TAB   48
#define VK_BS    51
#define VK_PRIOR 116
#define VK_NEXT  121
#define VK_HOME  115
#define VK_END   119
#define VK_LEFT  123
#define VK_UP    126
#define VK_RIGHT 124
#define VK_DOWN  125
#define VK_F1    122
#define VK_F2    120
#define VK_F3     99
#define VK_F4    118
#define VK_F5     96
#define VK_F6     97
#define VK_F7     98
#define VK_F8    100
#define VK_F9    101
#define VK_F10   109
#define VK_F11   103
#define VK_F12   111

#define KEY_SYM(name, sym) \
  if (key == VK_##name) \
    v = ID2SYM(rb_intern("" # sym)); \
  else
