/* 
 * This a Shoes edit_box
*/
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/types.h"
#include "shoes/native/cocoa/textview.h"
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

@implementation ShoesTextView
- (id)initWithFrame: (NSRect)frame andObject: (VALUE)o
{
  if ((self = [super initWithFrame: frame]))
  {
    object = o;
    attrs = NULL;
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

// ---- Ruby calls C/obj->C here ----

void
shoes_native_edit_box_set_text(SHOES_CONTROL_REF ref, char *msg)
{
  COCOA_DO(
    ShoesTextView *stv = (ShoesTextView *)ref;
    NSString *str = [[NSString alloc] initWithCString: msg encoding: NSUTF8StringEncoding];
    if (stv->attrs) {
      // new with shoes 3.3.6
      NSAttributedString *astr = [[NSAttributedString alloc] initWithString: str attributes: stv->attrs];
      [[stv textStorage] setAttributedString: astr];
    } else {
      // no attributes 
      [[[stv textStorage] mutableString] setString: str];
    }
  );
}

SHOES_CONTROL_REF
shoes_native_edit_box(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  INIT;
  char *fntstr = 0;
  VALUE fgclr = Qnil; // Could be hex color string or Shoes color object
  NSInteger fsize = 0;
  NSMutableString *fontname = [[NSMutableString alloc] initWithCapacity: 40];

  NSArray *fontsettings;
  ShoesTextView *tv = [[ShoesTextView alloc] initWithFrame:
    NSMakeRect(place->ix + place->dx, place->iy + place->dy,
    place->ix + place->dx + place->iw, place->iy + place->dy + place->ih)
    andObject: self];
    
  // local ref 
  //NSTextView *textView = tv.textView;
    
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
    NSMutableDictionary *dict = [[NSMutableDictionary alloc] initWithCapacity: 5];
    NSColor *clr;
    //NSString *title = [NSString stringWithUTF8String: msg];
    if (fntstr) {
      NSFont *font = [NSFont fontWithName: fontname size: fsize];
      if (font == NULL) {
        // Don't do this: rb_raise(rb_eArgError, "Font \"%s\" not found", fntstr);
        font = [NSFont fontWithName: @"arial" size: 12];
      }
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
        clr = [NSColor colorWithCalibratedRed: rg green: gb blue: bb alpha: 1.0];
        [dict setObject: clr forKey: NSForegroundColorAttributeName];
      }
    } 
    // save the dict so delegate has access as needed
    tv->attrs = dict;
    //textView.typingAttributes = dict; // not working.
  }
  shoes_native_edit_box_set_text((SHOES_CONTROL_REF )tv, msg);
  
  // Tooltip
  VALUE vtip = shoes_hash_get(attr, rb_intern("tooltip"));
  if (! NIL_P(vtip)) {
    char *cstr = RSTRING_PTR(vtip);
    NSString *tip = [NSString stringWithUTF8String: cstr];
    [tv setToolTip:tip];
  } 
  
  RELEASE;
  return (NSControl *)tv;
}

VALUE
shoes_native_edit_box_get_text(SHOES_CONTROL_REF ref)
{
  VALUE text = Qnil;
  INIT;
  text = rb_str_new2([[[(ShoesTextView *)ref textStorage] string] UTF8String]);
  RELEASE;
  return text;
}


void
shoes_native_edit_box_append(SHOES_CONTROL_REF ref, char *msg)
{
  COCOA_DO([[[(ShoesTextView *)ref textStorage] mutableString] appendString: [NSString stringWithUTF8String: msg]]);
}

void
shoes_native_edit_box_scroll_to_end(SHOES_CONTROL_REF ref)
{

  NSTextView *tv = [(ShoesTextView *) ref textView];
  [tv scrollRangeToVisible: NSMakeRange(tv.string.length, 0)];
}
