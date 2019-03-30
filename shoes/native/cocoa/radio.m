// Shoes Radio ssubclasses NSButton.

#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/types.h"
#include "shoes/native/cocoa/radio.h"
extern VALUE cTimer;

//#import <Carbon/Carbon.h>

#define HEIGHT_PAD 6

#define INIT    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]
#define RELEASE [pool release]
#define COCOA_DO(statements) do {\
  INIT; \
  @try { statements; } \
  @catch (NSException *e) { ; } \
  RELEASE; \
} while (0)

// forward declares
VALUE shoes_native_radio_clicked(VALUE self, VALUE group, VALUE state);
NSString *shoes_native_radio_imgPath(char *file);
void shoes_native_radio_set(VALUE ele, int on);

// globals
NSImage *onImg = NULL;
NSImage *offImg = NULL;

// ---- Cocoa Object side ----

// Radios are pretty broken for Shoes purposes.
// SHOES_FORCE_RADIO simulates radios using NSPushOnPushOffButton
// That make a visually clunkly apperance. 

@implementation ShoesRadioButton
- (id)initWithType: (NSButtonType)t andObject: (VALUE)o andGroup: (VALUE)group
{
  if ((self = [super init]))
  {
    object = o;
    groupArray = group;
    //[self setButtonType: NSPushOnPushOffButton]; 
    [self setButtonType: t];  
    [self setImagePosition: NSImageOnly];
    if (onImg == NULL) {
			NSString *onPath = shoes_native_radio_imgPath("/static/RadioButton-Selected.png");
	    onImg = [[NSImage alloc] initWithContentsOfFile: onPath];
	  }
	  [self setAlternateImage: onImg];
	  if (offImg == NULL) {
			NSString *offPath = shoes_native_radio_imgPath("/static/RadioButton-Unselected.png");
			offImg = [[NSImage alloc] initWithContentsOfFile: offPath];
		}
	  [self setImage: offImg];
    [self setBezelStyle: NSCircularBezelStyle];
    [self setTarget: self];
    [self setAction: @selector(handleClick:)];		
  }
  return self;
}
-(IBAction)handleClick: (id)sender
{
  //NSLog(@"radio button handler called on %lx", object);
  //shoes_button_send_click(object);

	shoes_native_radio_clicked(object, groupArray, Qtrue);
  // was: shoes_check_set_checked_m(object, Qtrue);
}
@end

// ---- Ruby calls C/obj->C here ----
SHOES_CONTROL_REF
shoes_native_radio(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, VALUE group)
{
  INIT;
/*
	ShoesButton *button = [[ShoesButton alloc] initWithType: NSRadioButton
		   andObject: self];
	 RELEASE;
	 return (NSControl *)button;
*/
/*
 	NSLog(@"shoes_native_radio self: %i, %lx", TYPE(self), self);
 	if (NIL_P(group))
 		NSLog(@"group: NIL");
   else
 	  NSLog(@"group: %lx", group);
*/

 	ShoesRadioButton *button = [[ShoesRadioButton alloc] initWithType: NSToggleButton
 		   andObject: self andGroup: group];

	// was ShoesRadioButton *button = [[ShoesRadioButton alloc] initWithType: NSRadioButton
 	//	   andObject: self andGroup: group];
  VALUE ck = shoes_hash_get(attr, rb_intern("checked"));
  if (!NIL_P(ck)) {
    //fprintf(stderr, "CHECKED!\n");
    [button setState: ((ck == Qtrue) ? NSOnState : NSOffState)];
  }
  // Tooltip
  VALUE vtip = shoes_hash_get(attr, rb_intern("tooltip"));
  if (! NIL_P(vtip)) {
    char *cstr = RSTRING_PTR(vtip);
    NSString *tip = [NSString stringWithUTF8String: cstr];
    [button setToolTip:tip];
  } 
 	RELEASE;
 	return (NSControl *)button;
}

void shoes_native_radio_set(VALUE ele, int on)
{
  Get_TypedStruct2(ele, shoes_control, ctl);
  ATTRSET(ctl->attr, checked, on);
  COCOA_DO([(ShoesRadioButton *)ctl->ref setState: on ? NSOnState : NSOffState]);
}

VALUE shoes_native_radio_clicked(VALUE self, VALUE group, VALUE state) {
  if (RTEST(state)) {
      if (!NIL_P(group)) {
          long i;
          for (i = 0; i < RARRAY_LEN(group); i++) {
              VALUE ele = rb_ary_entry(group, i);
              if (ele == self) 
								shoes_native_radio_set(ele, true);
							else
							  shoes_native_radio_set(ele, false);
          }
      }
  }
  return state;
}
NSString *shoes_native_radio_imgPath(char *file) {
  /* get the shoes/ruby DIR constant, convert to NSString, append argument */
  VALUE dir = rb_eval_string("DIR");
  char buf[256];
  strcpy(buf,RSTRING_PTR(dir));
  int pos = strlen(buf);
  strcpy(buf+pos, file);
  //fprintf(stderr, "path is %s\n", buf);
  return [[NSString alloc] initWithUTF8String: buf];
}


