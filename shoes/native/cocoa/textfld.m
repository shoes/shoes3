#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/types.h"
#include "shoes/native/cocoa/textfld.h"
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

// ---- Cocoa Object side ----

@implementation ShoesTextField
- (id)initWithFrame: (NSRect)frame andObject: (VALUE)o
{
  if ((self = [super initWithFrame: frame]))
  {
    object = o;
    [self setBezelStyle: NSRegularSquareBezelStyle];
    [self setDelegate: (id<NSTextFieldDelegate>)self];
  }
  //printf("Create %u\n",self);
  return self;
}
-(void)textDidChange: (NSNotification *)note
{
  //printf("didChange %u\n", self);
  shoes_control_send(object, s_change);
}

//  Shoes4 bug860, shoes3.2 bug008
/*
  Pay attention to the delicate delegate dance need to implement textShouldEndEditing
  if we implement textDidEndEditing. 'should' is called before 'end'
*/

-(void)textDidEndEditing: (NSNotification *)note
{
    //printf("didEndEditing %u\n", self);
	shoes_control_send(object, s_donekey);
}

// fixes bug that disappeared entered text
- (BOOL)textShouldEndEditing:(NSText *)textObject
{
  NSString *tmp = [textObject string];
  //printf("shouldEndEditing %u %s\n", self, [tmp UTF8String]);
  // need to save the contents
  [self setStringValue: tmp]; // yes, it seems silly but it works.
  return YES;
}
@end

@implementation ShoesSecureTextField
- (id)initWithFrame: (NSRect)frame andObject: (VALUE)o
{
  if ((self = [super initWithFrame: frame]))
  {
    object = o;
    [self setBezelStyle: NSRegularSquareBezelStyle];
    [self setDelegate: (id<NSTextFieldDelegate>)self];
  }
  return self;
}
-(void)textDidChange: (NSNotification *)n
{
  shoes_control_send(object, s_change);
}
@end

// ---- Ruby calls C/obj->C here ----

SHOES_CONTROL_REF
shoes_native_edit_line(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  INIT;
  NSTextField *field;
  char *fntstr = 0;
  VALUE fgclr = Qnil; // Could be hex color string or Shoes color object
  NSInteger fsize = 0;
  NSArray *fontsettings;
  NSMutableString *fontname = [[NSMutableString alloc] initWithCapacity: 40];
  if (RTEST(ATTR(attr, secret))) {
    field = [[ShoesSecureTextField alloc] initWithFrame:
      NSMakeRect(place->ix + place->dx, place->iy + place->dy,
      place->ix + place->dx + place->iw, place->iy + place->dy + place->ih)
      andObject: self];
  } else {
    field = [[ShoesTextField alloc] initWithFrame:
      NSMakeRect(place->ix + place->dx, place->iy + place->dy,
      place->ix + place->dx + place->iw, place->iy + place->dy + place->ih)
      andObject: self];
  
  }   
  if (!NIL_P(shoes_hash_get(attr, rb_intern("stroke")))) {
    fgclr = shoes_hash_get(attr, rb_intern("stroke"));
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
	    field.textColor = clr; // it's a Property
	  }
  }
  // get the Shoes font attributes  if any and get the osx equivalent
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
    if (fntstr) {
      NSFont *font = [NSFont fontWithName: fontname size: fsize];
      field.font = font;
    }
  }
  [field setStringValue: [NSString stringWithUTF8String: msg]];
  [field setEditable: YES];
  
  // Tooltip
  VALUE vtip = shoes_hash_get(attr, rb_intern("tooltip"));
  if (! NIL_P(vtip)) {
    char *cstr = RSTRING_PTR(vtip);
    NSString *tip = [NSString stringWithUTF8String: cstr];
    [field setToolTip:tip];
  } 
  RELEASE;
  return (NSControl *)field;
}

VALUE
shoes_native_edit_line_get_text(SHOES_CONTROL_REF ref)
{
  VALUE text = Qnil;
  INIT;
  text = rb_str_new2([[ref stringValue] UTF8String]);
  RELEASE;
  return text;
}

void
shoes_native_edit_line_set_text(SHOES_CONTROL_REF ref, char *msg)
{
  COCOA_DO([ref setStringValue: [NSString stringWithUTF8String: msg]]);
}

VALUE
shoes_native_edit_line_cursor_to_end(SHOES_CONTROL_REF ref)
{
  // TODO:
  return Qnil;
}
