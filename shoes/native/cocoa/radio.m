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


// ---- Cocoa Object side ----

// Radios are pretty broken for Shoes purposes.
// SHOES_FORCE_RADIO simulates radios using NSPushOnPushOffButton
// That make a visually clunkly apperance. 

@implementation ShoesRadioButton
- (id)initWithType: (NSButtonType)t andObject: (VALUE)o andGroup: (VALUE) group
{
  if ((self = [super init]))
  {
    object = o;
    groupArray = group;
    //[self setButtonType: NSPushOnPushOffButton];  // checkbox for now
    [self setButtonType: t];  
	  //[self setShoesViewPosition: NSImageOnly]; 
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
#ifdef SHOES_FORCE_RADIO
	shoes_native_radio_clicked(object, groupArray, Qtrue);
#else
  shoes_check_set_checked_m(object, Qtrue);
#endif
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
#ifdef SHOES_FORCE_RADIO
 	ShoesRadioButton *button = [[ShoesRadioButton alloc] initWithType: NSPushOnPushOffButton
 		   andObject: self andGroup: group];
#else 
	ShoesRadioButton *button = [[ShoesRadioButton alloc] initWithType: NSRadioButton
 		   andObject: self andGroup: group];
#endif
 	RELEASE;
 	return (NSControl *)button;
}

#ifdef SHOES_FORCE_RADIO
void shoes_native_radio_set(VALUE ele, int on)
{
	shoes_control *ctl;
  Data_Get_Struct(ele, shoes_control, ctl);
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
#endif
