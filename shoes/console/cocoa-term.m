/* cocoa-term is a very minimal terminal emulator for use in Shoes, 
   mostly for logging purposes. This is not a window that you can manage
   from Shoes.  
   Gui wise it is a Window with a small panel on top that has a static message
   and two buttons (copy and clear) and the a larger scrollable panel of
   text lines. There is a minimal keyboard support.
   
   Stdout/Stderr are re-implemnted using pipes instead of a pty
   because OSX is just damn weird about stdout depending on how you
   launch Shoes. Since pipes can not be line bufffered, we need to call
   flush when needed. The code here does not r/w from stdio/out/err except
   for diagnostic purposes. fd[0,1,2]] have their C/Objective-C/Ruby expected
   definitions.  Sadly, we have to tell Ruby that they have changed since 
   the Ruby init process grabbed the old (bad on OSX) fd's. 
   
   To confuse you more, we use a Pty for sending chars (keystrokes) to
   stdin.
   
   We have to subclass NSTextView (see comments in cocoa-term.h)
   
*/
#include "cocoa-term.h"
extern char *colorstrings[];

/* there can only be one terminal so only one tesi struct.
 I'll keep a global Obj-C ref to tesi and to the very odd bridge object
*/
static struct tesiObject* shadow_tobj;
static StdoutBridge *bridge = NULL;
int osx_cshoes_launch = 0; 		// extern in app.h

@implementation StdoutBridge
- (StdoutBridge *) init
{
  if (osx_cshoes_launch) {
    return self;
  }
  //printf("old stdout\n");
  oldHandle = [NSFileHandle fileHandleWithStandardOutput];
  NSFileHandle *nullHandle = [NSFileHandle fileHandleWithNullDevice];
  if (oldHandle == nullHandle) {
    printf("This is stdout = dev/null\n");
  }
  //char *pipeWrStr = "saved handle\n";
  //NSData *pipeMsgData = [NSData dataWithBytes: pipeWrStr length: strlen(pipeWrStr)];
  //[oldHandle writeData: pipeMsgData];
  // OSX Stdout is weird. This is too.
  int pipewrfd, piperdfd;
  int rtn = 0;
  outPipe = [NSPipe pipe];
  outReadHandle = [outPipe fileHandleForReading] ;
  outWriteHandle = [outPipe fileHandleForWriting];
  pipewrfd = [[outPipe fileHandleForWriting] fileDescriptor];
  piperdfd = [[outPipe fileHandleForReading] fileDescriptor];
  // fclose(stdout) // Don't do this!!
  rtn = dup2(pipewrfd, fileno(stdout));
  //write(fileno(stdout), "new fd\n", 7);
  //printf("new stdout\n");
  //fprintf(stderr,"At least we have stderr?\n");
  
 
  return self;
}

- (void) setSink
{
  [[NSNotificationCenter defaultCenter] addObserver: self
                                        selector: @selector(stdOutDataSink:)
                                        name: NSFileHandleDataAvailableNotification
                                        object: outReadHandle];
  [outReadHandle waitForDataInBackgroundAndNotify];}

- (void) removeSink
{
  [[NSNotificationCenter defaultCenter] removeObserver: self
										name: NSFileHandleDataAvailableNotification
									    object: outReadHandle];
}

- (void) stdOutDataSink: (NSNotification *) notification
{
  NSFileHandle *fh = (NSFileHandle *) [notification object];
  NSData *data = [fh availableData];
  int len = [data length];
  if (len) {
     // write data to oldHandle - what ever stdout OSX gave us.
     // Might be /dev/null or the launch terminal
    [oldHandle writeData: data];
    [outReadHandle waitForDataInBackgroundAndNotify];
  } else {
    // eof? close?
  }
}
@end

// create the bridge object before Ruby is initialized called from
// world.c 
void shoes_osx_setup_stdout() {
  // Temporarily disable stdout redirection for debugging
  bridge = NULL;
  return;
  
  if (osx_cshoes_launch) {
    bridge = NULL;
  } else {
    bridge = [[StdoutBridge alloc] init];
  }
}

/*  
 * this is called when event loop is started (cocoa.m )
*/
void shoes_osx_stdout_sink() {
  if (bridge == NULL) {
    return;  
  }
  [bridge setSink];
}

@implementation DisplayView

- (void)keyDown: (NSEvent *)e
{
#ifdef HALF_PTY
  /* 
   * This gets pretty weird - readline probably expects a canical terminal
   * setting where the pty does the line editing/echoing only
   * we don't have a pty stdout/stderr. SO we have to maintain a 'line'
   * buffer that matches the on screen view and then write that to
   * to the pty fd when \n arrives so readline gets it.
   *
   * Since textStorage buffer is actually correct could track the
   * line start and end points in buffer OR manage a duplicate which
   * is probably easier so thats what I do.
   * 
   */
  NSString *str = [e charactersIgnoringModifiers];
  char *utf8 = [str UTF8String];
  char ch = utf8[0];
  if (strlen(utf8)==1) {
    TerminalWindow *tw = (TerminalWindow *) shadow_tobj->pointer;
    // OSX delete key produces 0x7f but we want 0x8
    if (ch == 0x7f) { 
      // delete/bs key - because it's OSX!
      int length = [[[tw->termView textStorage] string] length];
      [[tw->termView textStorage] deleteCharactersInRange:NSMakeRange(length-1, 1)];
      [tw->termView scrollRangeToVisible:NSMakeRange([[tw->termView string] length], 0)];
      if (tw->linePos > 0) tw->lineBuffer[--tw->linePos] = '\0';
    } else if (ch == '\t') {
      tw->lineBuffer[tw->linePos++] = ' '; // better one than a computation of spaces.
      [tw displayChar: ch];
    } else if (ch == '\r') {
      // not conical - remember? 
      tw->lineBuffer[tw->linePos++] = '\r';
      //write(shadow_tobj->fd_input, tw->lineBuffer, strlen(tw->lineBuffer));
      write(shadow_tobj->fd_input, tw->lineBuffer, tw->linePos);
      [tw displayChar: ch];
      tw->linePos = 0;
      tw->lineBuffer[0] = '\0';
    } else {
      tw->lineBuffer[tw->linePos++] = ch; 
      [tw displayChar: utf8[0]];
    }
  } else {
    // this sends the key event (back?) to the responder chain
    [self interpretKeyEvents:[NSArray arrayWithObject:e]];
  }
#endif
}

@end

@implementation TerminalWindow

- (void)displayChar:(char)c
{
  char buff[4];
  buff[0] = c;
  buff[1] = 0;
  NSString *cnvbfr = [[NSString alloc] initWithCString: buff encoding: NSUTF8StringEncoding];
  //Create a AttributeString using the font, fg color...
  NSAttributedString *attrStr = [[NSAttributedString alloc] initWithString: cnvbfr attributes: attrs];
  [[termView textStorage] appendAttributedString: attrStr];
  [termView scrollRangeToVisible:NSMakeRange([[termView string] length], 0)];
  [attrStr release];
  [cnvbfr release];
  // TODO: Am I leaking memory ? The C programmer in me says "Oh hell yes!"
  // [obj release] to match the alloc's ?
}

- (void)initAttributes 
{
  colorTable = [NSMutableDictionary dictionaryWithCapacity: 9];
  [colorTable setObject: [NSColor blackColor] forKey: @"black"];
  [colorTable setObject: [NSColor redColor] forKey: @"red"];
  [colorTable setObject: [NSColor greenColor] forKey: @"green"];
  [colorTable setObject: [NSColor brownColor] forKey: @"brown"];
  [colorTable setObject: [NSColor blueColor] forKey: @"blue"];
  [colorTable setObject: [NSColor magentaColor] forKey: @"magenta"];
  [colorTable setObject: [NSColor cyanColor] forKey: @"cyan"];
  [colorTable setObject: [NSColor whiteColor] forKey: @"white"];
  [colorTable setObject: [NSColor yellowColor] forKey: @"yellow"];
#if 0
  colorAttr = [[NSArray alloc] initWithObjects:
      [NSColor blackColor],[NSColor redColor],[NSColor greenColor],
      [NSColor brownColor],[NSColor blueColor],[NSColor magentaColor],
      [NSColor cyanColor], [NSColor whiteColor], nil];
#else
  colorAttr = [[NSMutableArray alloc]  initWithCapacity: 256];
  int i;
  for (i = 0; i < 256; i++) {
    char *hashc = strchr(colorstrings[i], '#');
    int rgb;
    int r, g, b;
    sscanf(++hashc,"%x", &rgb);
    b = rgb & 255; rgb = rgb >> 8;
    g = rgb & 255; rgb = rgb >> 8;
    r = rgb & 255;
    CGFloat rg = (CGFloat)r / 255;
    CGFloat gb = (CGFloat)g / 255;
    CGFloat bb = (CGFloat)b / 255;
    NSColor *clr = [NSColor colorWithCalibratedRed: rg green: gb blue: bb alpha: 1.0];
    [colorAttr insertObject: clr atIndex: i];
  }
#endif
}


- (void)consoleInitWithFont: (NSFont *)font app_dir: (char *)app_dir 
      mode: (int)mode columns: (int)columns rows: (int)rows foreground: (char *)fg
      background: (char *)bg title: (char *)title
{
  monoFont = font;
  monoBold = [[NSFontManager sharedFontManager] 
      convertFont: font
      toHaveTrait: NSBoldFontMask];
  boldActive = FALSE;
  req_mode = mode;
  req_cols = columns;
  req_rows = rows;
  [self initAttributes];
  // TODO: fg and bg are really Shoes colors names so we should ask Shoes
  // for the cocoa color object; 
  defaultBgColor = [NSColor whiteColor];
  if (bg != NULL) {
    defaultBgColor = [colorTable objectForKey: [[NSString alloc] initWithUTF8String: bg]];
  }
  defaultFgColor = [NSColor blackColor];
  if (fg != NULL) {
    defaultFgColor = [colorTable objectForKey: [[NSString alloc] initWithUTF8String: fg]];
  }
  attrs = [[NSMutableDictionary alloc] init];
  [attrs setObject: monoFont forKey: NSFontAttributeName];
  [attrs setObject: defaultFgColor  forKey: NSForegroundColorAttributeName];

  NSSize charSize = [monoFont maximumAdvancement];
  float fw = charSize.width;
  float fh = [monoFont pointSize]+2.0;
  int width = (int)(fw * (req_cols + 8));
  int height = (int)(fh * req_rows);
  int btnPanelH = 40;
  
  NSString *reqTitle = [[NSString alloc] initWithUTF8String: title];
  [self setTitle: reqTitle];
  [self makeKeyAndOrderFront: self];
  [self setAcceptsMouseMovedEvents: YES];
  [self setAutorecalculatesKeyViewLoop: YES];
  [self setDelegate: (id <NSWindowDelegate>)self];
  
  // setup the copy and clear buttons (yes command key handling would be better)
  btnpnl = [[NSBox alloc] initWithFrame: NSMakeRect(0,height,width,btnPanelH)];
  [btnpnl setTitlePosition: NSNoTitle ];
  [btnpnl setAutoresizingMask: NSViewWidthSizable|NSViewMinYMargin];
  
  // draw the icon
  NSApplication *NSApp = [NSApplication sharedApplication];
  NSImage *icon = [NSApp applicationIconImage];
  NSRect iconRect = NSMakeRect(20,-2,32,32); // -2 doesn't make sense but ...
  NSImageView *ictl = [[NSImageView alloc] initWithFrame: iconRect];
  [ictl setImage: icon];
  [ictl setEditable: false];

  NSTextField *labelWidget;
  labelWidget = [[NSTextField alloc] initWithFrame: NSMakeRect(80, -2, 200, 28)];
  [labelWidget setStringValue: reqTitle];
  [labelWidget setBezeled:NO];
  [labelWidget setDrawsBackground:NO];
  [labelWidget setEditable:NO];
  [labelWidget setSelectable:NO];
  NSFont *labelFont = [NSFont fontWithName:@"Helvetica" size:18.0];
  [labelWidget setFont: labelFont];

  clrbtn = [[NSButton alloc] initWithFrame: NSMakeRect(300, -2, 60, 28)];
  [clrbtn setButtonType: NSMomentaryPushInButton];
  [clrbtn setBezelStyle: NSRoundedBezelStyle];
  [clrbtn setTitle: @"Clear"];
  [clrbtn setTarget: self];
  [clrbtn setAction: @selector(handleClear:)];

  cpybtn = [[NSButton alloc] initWithFrame: NSMakeRect(400, -2, 60, 28)];
  [cpybtn setButtonType: NSMomentaryPushInButton];
  [cpybtn setBezelStyle: NSRoundedBezelStyle];
  [cpybtn setTitle: @"Copy"];
  [cpybtn setTarget: self];
  [cpybtn setAction: @selector(handleCopy:)];
  
  rawbtn = [[NSButton alloc] initWithFrame: NSMakeRect(480, -2, 100, 28)];
  [rawbtn setButtonType: NSMomentaryPushInButton];
  [rawbtn setBezelStyle: NSRoundedBezelStyle];
  [rawbtn setTitle: @"copy raw"];
  [rawbtn setTarget: self];
  [rawbtn setAction: @selector(handleRawCopy:)];

  [btnpnl addSubview: ictl];
  [btnpnl addSubview: labelWidget];
  [btnpnl addSubview: clrbtn];
  [btnpnl addSubview: cpybtn];
  [btnpnl addSubview: rawbtn];
  
  // init termpnl and textview here.
  // Note NSTextView is subclass of NSText so there are MANY methods to learn
  // along with delegates and protocols. It's also very MVC.

  // compute the Size of the window for the font and size
  NSRect textViewBounds = NSMakeRect(0, 0, width, height);

  // setup internals for NSTextView  - storage(model) and Layout(control)
  termStorage = [[NSTextStorage alloc] init];
  [termStorage setFont: monoFont];
  termLayout = [[NSLayoutManager alloc] init];
  [termStorage addLayoutManager:termLayout];
  termContainer = [[NSTextContainer alloc] initWithContainerSize:textViewBounds.size];
  [termLayout addTextContainer:termContainer];
  
  /*
   * Now deal with the View part of MVC. Scrolling adds confusion. 
   * According to Apple, you setup the NSScrollView, then the NSTextView to
   * fit that.
  */
  // Bump the width by the size of a Vertical scollbar ?
  //width += [NSScroller scrollerWidth]; 
  
  termView = [[DisplayView alloc]  initWithFrame: NSMakeRect(0, 0, width, height)];
  termView.backgroundColor =  defaultBgColor; // fun with Properties!!
  termView.drawsBackground = true;
  [termView setEditable: YES];
  [termView setRichText: false];

  termpnl = [[NSScrollView alloc] initWithFrame: NSMakeRect(0, 0, width, height)];
  [termpnl setHasVerticalScroller: YES];
  [termpnl setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
  [termpnl setDocumentView: termView];

  // Put the panels in the Window
  cntview = [[NSView alloc] initWithFrame: NSMakeRect(0,height+btnPanelH,width,height+btnPanelH)];
  [self setContentView: cntview];
  [cntview setAutoresizesSubviews: YES];
  [cntview addSubview: btnpnl];
  [cntview addSubview: termpnl];
  [self makeFirstResponder:termView];
  //[termView initView: self withFont: monoFont]; // tell ConsoleTermView what font to use
  
  /* -- done with most of visual setup -- now for the confusing io setup. */
  
  errPipe = [NSPipe pipe];
  errReadHandle = [errPipe fileHandleForReading];
  if (dup2([[errPipe fileHandleForWriting] fileDescriptor], fileno(stderr)) != -1) {
    [[NSNotificationCenter defaultCenter] addObserver: self
                                        selector: @selector(stdErrDataAvail:)
                                        name: NSFileHandleDataAvailableNotification
                                        object: errReadHandle];
    [errReadHandle waitForDataInBackgroundAndNotify];
  }
  // Is there a stdout bridge setup ? Probably not, so set it up.
  if (! bridge) {
    osx_cshoes_launch = 0;
    bridge = [[StdoutBridge alloc] init];
    [bridge setSink];
  }
  // Replace the bridge's sink observer with one that puts the chars on the
  // new window
  outReadHandle = bridge->outReadHandle;
  [bridge removeSink];
  
#if 1
  [[NSNotificationCenter defaultCenter] addObserver: self
                                        selector: @selector(stdOutDataAvail:)
                                        name: NSFileHandleDataAvailableNotification
                                        object: outReadHandle];
  [bridge->outReadHandle waitForDataInBackgroundAndNotify];
#else
   [[NSNotificationCenter defaultCenter] addObserver: self
                                        selector: @selector(handleNotification:)
                                        name: NSFileHandleReadCompletionNotification
                                        object: outReadHandle];
   [outReadHandle readInBackgroundAndNotify] ;
#endif  
    // For debugging init a buffer
     rawBuffer = [[NSMutableData alloc] initWithCapacity: 2048];
    // now convince Ruby to use the new stdout - sadly it's not line buffered
    // BEWARE the monkey patch!
    char evalstr[256];
#if 1
    strcpy(evalstr, 
      "class IO \n\
          def puts (args=nil) \n\
            super args \n\
            self.flush \n\
          end \n\
        end\n");
   rb_eval_string(evalstr);
#endif
  // create a buffer for input line chars
  lineBuffer = malloc(req_cols);
  linePos = 0;
  // Now init the Tesi object - NOTE tesi callbacks are C,  which calls Objective-C
  tobj = newTesiObject("/bin/bash", req_cols, req_rows); // first arg is not used
  shadow_tobj = tobj; 
  tobj->pointer = (void *)self;
  //tobj->callback_haveCharacter = &console_haveChar; // short circuit - to be deleted
  tobj->callback_handleNL = &terminal_newline;
  tobj->callback_handleRTN = NULL; // &terminal_return;
  tobj->callback_handleBS = &terminal_backspace;
  tobj->callback_handleTAB = &terminal_tab; 
  tobj->callback_handleBEL = NULL;
  tobj->callback_printCharacter = &terminal_visAscii;
  tobj->callback_attreset = &terminal_attreset;
  tobj->callback_charattr = &terminal_charattr;
  tobj->callback_setfgcolor= &terminal_setfgcolor;
  tobj->callback_setbgcolor = &terminal_setbgcolor;
  tobj->callback_setfg256= &terminal_setfg256;
  tobj->callback_setbg256 = &terminal_setbg256;
  // that's the minimum set of call backs;
  tobj->callback_setdefcolor = NULL;
  tobj->callback_deleteLines = NULL;
  tobj->callback_insertLines = NULL;
  tobj->callback_attributes = NULL; // old tesi - not used? 
  
  tobj->callback_clearScreen = NULL;  //&terminal_clearscreen;
  tobj->callback_eraseCharacter = NULL; // &console_eraseCharacter;
  tobj->callback_moveCursor = NULL; //&terminal_moveCursor; 
  tobj->callback_insertLines = NULL; //&console_insertLine;
  tobj->callback_eraseLine = NULL; // &terminal_eraseLine;
  tobj->callback_scrollUp = NULL; // &console_scrollUp;

  // try inserting some text. Debugging purposes
  // [[termView textStorage] appendAttributedString: [[NSAttributedString alloc] initWithString: @"First Line!\n"]];
  
#if 0
  // need to get the handleInput started
  // OSX timer resolution less than 0.1 second unlikely?
  cnvbfr = [[NSMutableString alloc] initWithCapacity: 4];
  pollTimer = [NSTimer scheduledTimerWithTimeInterval:0.1
                            target: self selector:@selector(readStdout:)
                            userInfo: self repeats:YES];
  // debug
#endif
  /*
  * outfd = fileno(stdout);
  * fprintf(stderr, "About C stdout after:\n");
  * fprintf(stderr, "fd: %d, pipewrfd: %d, piperdfd: %d\n", outfd, pipewrfd, piperdfd);
  * fprintf(stderr, "dup2 errorno: %d, %s\n", dup2fail, strerror(dup2fail));
  
  * fprintf(stdout, "From C stdout\n");  // Nothing, Damn it!
  * // printf("w = %d, h = %d winh = %d \n", width, height, winRect.size.height);
  */
}

-(IBAction)handleClear: (id)sender
{
  [termView setString: @""];
  [rawBuffer release];
  rawBuffer = [[NSMutableData alloc] initWithCapacity: 2048];
}

-(IBAction)handleCopy: (id)sender
{
  NSString *str = [termView string];
  [[NSPasteboard generalPasteboard] declareTypes: [NSArray arrayWithObject: NSStringPboardType] owner: nil];
  [[NSPasteboard generalPasteboard] setString: str forType: NSStringPboardType]; 
}

-(IBAction)handleRawCopy: (id)sender
{
  NSString *str = [[NSString alloc] initWithData: rawBuffer encoding: NSUTF8StringEncoding];
  [[NSPasteboard generalPasteboard] declareTypes: [NSArray arrayWithObject: NSStringPboardType] owner: nil];
  [[NSPasteboard generalPasteboard] setString: str forType: NSStringPboardType]; 
}

- (void)disconnectApp
{
  //app = Qnil;
}

/*
- (void)keyDown: (NSEvent *)e
{
  // works but I do not like it.
  NSString *str = [e charactersIgnoringModifiers];
  char *utf8 = [str UTF8String];
  if (strlen(utf8)==1) {
    write(tobj->fd_input, utf8, strlen(utf8));
  } else {
    // this sends the key event (back?) to the responder chain
    [self interpretKeyEvents:[NSArray arrayWithObject:e]];
  }
}
*/


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
}

/*
- (void)readStdout: (NSTimer *)t
{
  // do tesi.Input
  tesi_handleInput(tobj);
}

- (void)writeStr:(NSString*)text
{
  [[termView textStorage] appendAttributedString: [[NSAttributedString alloc] initWithString: text]];
  [termView scrollRangeToVisible:NSMakeRange([[termView string] length], 0)];
}
*/


- (void) stdOutDataAvail: (NSNotification *) notification
{
  NSFileHandle *fh = (NSFileHandle *) [notification object];
  NSData *data = [fh availableData];
  // Debug capture
  [rawBuffer appendData: data];
  int len = [data length];
  if (len) {
    char *s = (char *)[data bytes];  // odds are high this is UTF16-LE
    s[len] = '\0';
    // feed  str to tesi and it will callback into other C code defined here.
    tesi_handleInput(tobj, s, len);
    
    //NSString *str = [[NSString alloc] initWithData: data encoding: NSASCIIStringEncoding] ;
    //[[termView textStorage] appendAttributedString: [[NSAttributedString alloc] initWithString: str]];
    //[termView scrollRangeToVisible:NSMakeRange([[termView string] length], 0)];
    
    [bridge->outReadHandle waitForDataInBackgroundAndNotify];
  } else {
    // eof? close?
  }
}

- (void) stdErrDataAvail: (NSNotification *) notification
{
  NSFileHandle *fh = (NSFileHandle *) [notification object];
  NSData *data = [fh availableData];
  int len = [data length];
  if (len) {
    char *s = (char *)[data bytes];  // odds are high this is UTF16-LE
    s[len] = '\0';
    // feed  str to tesi and it will callback into other C code defined here.
    tesi_handleInput(tobj, s, len);
    
    //NSString *str = [[NSString alloc] initWithData: data encoding: NSASCIIStringEncoding] ;
    //[[termView textStorage] appendAttributedString: [[NSAttributedString alloc] initWithString: str]];
    //[termView scrollRangeToVisible:NSMakeRange([[termView string] length], 0)];
    
    [fh waitForDataInBackgroundAndNotify];
  } else {
    // eof? close?
  }
}

@end

void shoes_native_terminal(char *app_dir, int mode, int columns, int rows,
    int fontsize, char* fg, char *bg, char* title) 
{
  
  NSFont *font = [NSFont fontWithName:@"Menlo" size: (double)fontsize]; //menlo is monospace
  NSSize charSize = [font maximumAdvancement];
  float fw = charSize.width;
  float fh = [font pointSize]+2.0;
  //int width = (int)(fw * 80.0);
  //int height = (int)(fh * 24);
  int width = fw * (columns+1);
  int height = fh * rows;
  int btnPanelH = 40; //TODO: dont hardcode this here
  TerminalWindow *window;
  unsigned int mask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
  NSRect rect = NSMakeRect(0, 0, width, height+btnPanelH); //Screen co-ords?
  //NSSize size = {app->minwidth, app->minheight};

  //if (app->resizable)
    mask |= NSResizableWindowMask;
  window = [[TerminalWindow alloc] initWithContentRect: rect
    styleMask: mask backing: NSBackingStoreBuffered defer: NO];
  //if (app->minwidth > 0 || app->minheight > 0)
  //  [window setContentMinSize: size];
  [window consoleInitWithFont: font app_dir: app_dir mode: mode columns: columns
      rows: rows foreground: fg background: bg title: title];

  printf("Mak\010c\t console \t\tcreated\n"); //test \b \t in string
  fflush(stdout); // OSX pipes are not line buffered
}

//void shoes_native_console() {
//  shoes_native_terminal(NULL, 1, 80, 24, 12, NULL, NULL, "Shoes" );
//}

void terminal_visAscii (struct tesiObject *tobj, char c, int x, int y) {
  TerminalWindow *cpanel = (TerminalWindow *)tobj->pointer;
  [cpanel displayChar: c];
  //ConsoleTermView *cwin = cpanel->termView;
  //[cwin writeChr: c];
}


void terminal_backspace(struct tesiObject *tobj, int x, int y) {
  TerminalWindow *tw = (TerminalWindow *)tobj->pointer;
  int length = [[[tw->termView textStorage] string] length];
  [[tw->termView textStorage] deleteCharactersInRange:NSMakeRange(length-1, 1)];
  [tw->termView scrollRangeToVisible:NSMakeRange([[tw->termView string] length], 0)];
  
  //ConsoleTermView *cwin = cpanel->termView;
  //[cwin deleteChar];
}

void terminal_newline (struct tesiObject *tobj, int x, int y) {
  TerminalWindow *cpanel = (TerminalWindow *)tobj->pointer;
  [cpanel displayChar: '\n'];
  //ConsoleTermView *cwin = cpanel->termView;
  //[cwin writeChr: '\n'];
}

void terminal_tab(struct tesiObject *tobj, int x, int y) {
  return terminal_visAscii(tobj, '\t', x, y);
}

// deal with terminal character attributes - we just update the attr hash
// used for inserting chars into the NSTextView and hope for the best. 
// TODO: these are called on the ConsoleTermView object not TerminalWindow

void terminal_setfgcolor(struct tesiObject *tobj, int fg) {
  NSColor *clr;
  TerminalWindow *cpanel = (TerminalWindow *)tobj->pointer;
  //ConsoleTermView *cwin = cpanel->termView;
  NSArray *clrtab = cpanel->colorAttr;
  clr = [clrtab objectAtIndex: (fg - 30)+8]; // use brigher color
  [cpanel->attrs setObject: clr forKey: NSForegroundColorAttributeName];
}

void terminal_setbgcolor(struct tesiObject *tobj, int bg) {
  NSColor *clr;
  TerminalWindow *cpanel = (TerminalWindow *)tobj->pointer;
  //ConsoleTermView *cwin = cpanel->termView;
  NSArray *clrtab = cpanel->colorAttr;
  clr = [clrtab objectAtIndex: (bg - 40)]; // use bright range +8?
  [cpanel->attrs setObject: clr forKey: NSBackgroundColorAttributeName];
}

void terminal_setfg256(struct tesiObject *tobj, int fg) {
  NSColor *clr;
  TerminalWindow *cpanel = (TerminalWindow *)tobj->pointer;
  //ConsoleTermView *cwin = cpanel->termView;
  NSArray *clrtab = cpanel->colorAttr;
  clr = [clrtab objectAtIndex: fg];
  [cpanel->attrs setObject: clr forKey: NSForegroundColorAttributeName];
}

void terminal_setbg256(struct tesiObject *tobj, int bg) {
  NSColor *clr;
  TerminalWindow *cpanel = (TerminalWindow *)tobj->pointer;
  //ConsoleTermView *cwin = cpanel->termView;
  NSArray *clrtab = cpanel->colorAttr;
  clr = [clrtab objectAtIndex: bg ];
  [cpanel->attrs setObject: clr forKey: NSBackgroundColorAttributeName];
}

// we only care about a few of the possible tags values like bold,
// underline. Might be tricky.

void terminal_charattr(struct tesiObject *tobj, int attr) {
  TerminalWindow *cpanel = (TerminalWindow *)tobj->pointer;
  // 1 => bold, 4  => underline
  if (attr == 4) {
    [cpanel->attrs setObject: [NSNumber numberWithInt:NSUnderlineStyleSingle] forKey: NSUnderlineStyleAttributeName]; 
  } else if (attr == 1) {
    // causes a crash using attributes:
    //[cpanel->attrs setObject: cpanel->monoBold forKey: NSFontAttributeName];
    
    // note the place in textStorage where bold begins
    cpanel->boldActive = true;
    cpanel->boldStart = [[[cpanel->termView textStorage] string] length] - 1 ;
  }
}

void terminal_attreset(struct tesiObject *tobj) {
  // reset all attibutes (color, bold,...)
  TerminalWindow *cpanel = (TerminalWindow *)tobj->pointer;
  //ConsoleTermView *cwin = cpanel->termView;
  [cpanel->attrs setObject: cpanel->defaultBgColor forKey: NSBackgroundColorAttributeName];
  [cpanel->attrs setObject: cpanel->defaultFgColor forKey: NSForegroundColorAttributeName];
  [cpanel->attrs removeObjectForKey: NSUnderlineStyleAttributeName];
  if (cpanel->boldActive) {
     int boldEnd = [[[cpanel->termView textStorage] string] length]; // off by one?
     NSRange rng = NSMakeRange(cpanel->boldStart, boldEnd - cpanel->boldStart);
     
     NSMutableAttributedString* text = [cpanel->termView textStorage];
     [text applyFontTraits:NSBoldFontMask range: rng];     
     cpanel->boldActive = false;
  }
  [cpanel->attrs setObject: cpanel->monoFont forKey: NSFontAttributeName];
}




