//
// shoes/ruby.h
// Some defs for Ruby.
//
#include <ruby.h>
#include "shoes/canvas.h"

#ifndef SHOES_RUBY_H
#define SHOES_RUBY_H

#if SHOES_WIN32
typedef VALUE (*HOOK)(...);
#define CASTHOOK(x) reinterpret_cast<HOOK>(x)
#else
typedef VALUE (*HOOK)();
#define CASTHOOK(x) x
#endif

#ifndef RARRAY_LEN
#define RARRAY_LEN(arr)  RARRAY(arr)->len
#define RSTRING_LEN(str) RSTRING(str)->len
#define RSTRING_PTR(str) RSTRING(str)->ptr
#endif

extern VALUE cShoes, cApp, cCanvas, cFlow, cStack, cMask, cNative, cPath, cVideo, cImage, cAnim, cPattern, cBorder, cBackground, cPara, cBanner, cTitle, cSubtitle, cTagline, cCaption, cInscription, cLinkText, cTextBlock, cTextClass, cSpan, cStrong, cSub, cSup, cCode, cDel, cEm, cIns, cButton, cEditLine, cEditBox, cListBox, cProgress, cColor, cColors, cLink;
extern VALUE reHEX_SOURCE, reHEX3_SOURCE, reRGB_SOURCE, reRGBA_SOURCE, reGRAY_SOURCE, reGRAYA_SOURCE;
extern ID s_aref, s_bind, s_new, s_run, s_to_pattern, s_to_i, s_to_s, s_angle, s_arrow, s_begin, s_call, s_center, s_change, s_click, s_corner, s_downcase, s_draw, s_end, s_font, s_hand, s_hidden, s_href, s_insert, s_items, s_leading, s_match, s_scroll, s_text, s_title, s_top, s_right, s_bottom, s_left, s_height, s_remove, s_resizable, s_strokewidth, s_width, s_margin, s_margin_left, s_margin_right, s_margin_top, s_margin_bottom, s_radius, s_secret;
extern VALUE instance_eval_proc, exception_proc, exception_alert_proc;

VALUE mfp_instance_eval(VALUE, VALUE);
long rb_ary_index_of(VALUE, VALUE);
VALUE rb_ary_insert_at(VALUE, long, int, VALUE);
VALUE shoes_safe_block(VALUE, VALUE, VALUE);
void shoes_ruby_init(void);

#define BEZIER 0.55228475;

//
// Exception handling strings for eval
//
#define SHOES_META \
  "(class << Shoes; self; end).instance_eval do;"
#define EXC_ALERT \
  "proc do; alert %{#{@exc.message}\\n#{@exc.backtrace.map { |x| %{\\n  * #{x}}}}}; end"
#define EXC_MARKUP \
  "title e.message; para %%{#{e.backtrace.map { |x| %%{\\n  * #{Shoes.escape(x)}} }}};"
#define EXC_PROC \
  "proc do;" \
    EXC_MARKUP \
  "end;"
#define EXC_RUN \
  "define_method :run do |path|;" \
    "p = " EXC_PROC \
    "[p];" \
  "end;"

#define NUM2RGBINT(x) (rb_obj_is_kind_of(x, rb_cFloat) ? NUM2DBL(x) * 255 : NUM2INT(x))
#define DEF_COLOR(name, r, g, b) rb_hash_aset(cColors, ID2SYM(rb_intern("" # name)), shoes_color_new(r, g, b, 255))

//
// Common funcs for dealing with attribute hashes
//
#define ATTR(attr, n)                  shoes_hash_get(attr, s_##n)
#define PX(attr, n, dn, pn)            shoes_px(attr, s_##n, dn, pn)
#define PX2(attr, n1, n2, dn, dr, pn)  shoes_px2(attr, s_##n1, s_##n2, dn, dr, pn)
#define ATTR2(typ, attr, n, dn)        shoes_hash_##typ(attr, s_##n, dn)
#define ATTRSET(attr, k, v)            attr = shoes_hash_set(attr, s_##k, v)
#define ATTR_MARGINS(attr, dm) \
  int margin = ATTR2(int, attr, margin, dm); \
  int lmargin = ATTR2(int, attr, margin_left, margin); \
  int rmargin = ATTR2(int, attr, margin_right, margin); \
  int tmargin = ATTR2(int, attr, margin_top, margin); \
  int bmargin = ATTR2(int, attr, margin_bottom, margin)

#define REL_WINDOW  1
#define REL_CANVAS  2
#define REL_CURSOR  3
#define REL_TILE    4

int shoes_px(VALUE, ID, int, int);
int shoes_px2(VALUE, ID, ID, int, int, int);
VALUE shoes_hash_set(VALUE, ID, VALUE);
VALUE shoes_hash_get(VALUE, ID);
int shoes_hash_int(VALUE, ID, int);
double shoes_hash_dbl(VALUE, ID, double);
char *shoes_hash_cstr(VALUE, ID, char *);
VALUE rb_str_to_pas(VALUE);
void shoes_place_decide(shoes_place *, VALUE, VALUE, int, int, char);
void shoes_cairo_rect(cairo_t *, double, double, double, double, double);

#define CANVAS_DEFS(f) \
  f("width=", set_width, 1); \
  f("width", get_width, 0); \
  f("height=", set_height, 1); \
  f("height", get_height, 0); \
  f("nostroke", nostroke, 0); \
  f("stroke", stroke, -1); \
  f("strokewidth", strokewidth, 1); \
  f("nofill", nofill, 0); \
  f("fill", fill, -1); \
  f("rect", rect, -1); \
  f("oval", oval, -1); \
  f("line", line, 4); \
  f("arrow", arrow, 3); \
  f("star", star, -1); \
  f("para", para, -1); \
  f("banner", banner, -1); \
  f("title", title, -1); \
  f("subtitle", subtitle, -1); \
  f("tagline", tagline, -1); \
  f("caption", caption, -1); \
  f("inscription", inscription, -1); \
  f("code", code, -1); \
  f("del", del, -1); \
  f("em", em, -1); \
  f("ins", ins, -1); \
  f("link", link, -1); \
  f("span", span, -1); \
  f("strong", strong, -1); \
  f("sub", sub, -1); \
  f("sup", sup, -1); \
  f("background", background, -1); \
  f("border", border, -1); \
  f("video", video, -1); \
  f("image", image, -1); \
  f("imagesize", imagesize, 1); \
  f("animate", animate, -1); \
  f("path", path, -1); \
  f("move_to", move_to, 2); \
  f("line_to", line_to, 2); \
  f("curve_to", curve_to, 6); \
  f("transform", transform, 1); \
  f("translate", translate, 2); \
  f("rotate", rotate, 1); \
  f("scale", scale, -1); \
  f("skew", skew, -1); \
  f("push", push, 0); \
  f("pop", pop, 0); \
  f("reset", reset, 0); \
  f("button", button, -1); \
  f("list_box", list_box, -1); \
  f("edit_line", edit_line, -1); \
  f("edit_box", edit_box, -1); \
  f("progress", progress, -1); \
  f("contents", contents, 0); \
  f("draw", draw, 1); \
  f("after", after, -1); \
  f("before", before, -1); \
  f("append", append, -1); \
  f("prepend", prepend, -1); \
  f("flow", flow, -1); \
  f("stack", stack, -1); \
  f("mask", mask, -1); \
  f("hide", hide, 0); \
  f("show", show, 0); \
  f("toggle", toggle, 0); \
  f("click", click, -1); \
  f("release", release, -1); \
  f("motion", motion, -1); \
  f("keypress", keypress, -1); \
  f("clear", clear_contents, -1); \
  f("visit", goto, 1); \
  f("remove", remove, 0); \
  f("mouse", mouse, 0); \
  f("clipboard", get_clipboard, 0); \
  f("clipboard=", set_clipboard, 1)

#endif
