#import <Cocoa/Cocoa.h>
@interface ShoesTextView : NSScrollView
{
@public
  VALUE object;
  NSTextView *textView;
  NSDictionary *attrs;
}
@end
