#include "shoes/types/text.h"
#include "shoes/types/text_link.h"
#include "shoes/app.h"

// ruby
VALUE cLink, cLinkText, cLinkHover, cLinkUrl;

#ifdef NEW_MACRO_APP
FUNC_T(".link", link, -1);
#else
FUNC_M(".link", link, -1);
#endif

EVENT_COMMON(linktext, text, click);
EVENT_COMMON(linktext, text, release);
EVENT_COMMON(linktext, text, hover);
EVENT_COMMON(linktext, text, leave);

void shoes_text_link_init() {
    cLink      = rb_define_class_under(cTypes, "Link", cTextClass);
    rb_define_method(cTextClass, "click", CASTHOOK(shoes_linktext_click), -1);
    rb_define_method(cTextClass, "release", CASTHOOK(shoes_linktext_release), -1);
    rb_define_method(cTextClass, "hover", CASTHOOK(shoes_linktext_hover), -1);
    rb_define_method(cTextClass, "leave", CASTHOOK(shoes_linktext_leave), -1);
    
    cLinkHover = rb_define_class_under(cTypes, "LinkHover", cTextClass);
#ifdef NEW_MACRO_LINK
    cLinkUrl = rb_define_class_under(cTypes, "LinkUrl", rb_cData);
#else
    cLinkUrl = rb_define_class_under(cTypes, "LinkUrl", rb_cObject);
#endif
    RUBY_M(".link", link, -1);
}

// ruby
void shoes_link_mark(shoes_link *link) {
}

void shoes_link_free(shoes_link *link) {
    RUBY_CRITICAL(free(link));
}

#ifdef NEW_MACRO_LINK
// creates struct shoes_link_type
TypedData_Type_New(shoes_link);
#undef GET_STRUCT
#endif

VALUE shoes_link_alloc(VALUE klass) {
    VALUE obj;
    shoes_link *link = SHOE_ALLOC(shoes_link);
    SHOE_MEMZERO(link, shoes_link, 1);
#ifdef NEW_MACRO_LINK
    obj= TypedData_Wrap_Struct(klass, &shoes_link_type, link);
#else
    obj = Data_Wrap_Struct(klass, shoes_link_mark, shoes_link_free, link);
#endif
    link->ele = Qnil;
    return obj;
}

VALUE shoes_link_new(VALUE ele, int start, int end) {
    VALUE obj = shoes_link_alloc(cLinkUrl);
#ifdef NEW_MACRO_LINK
    Get_TypedStruct2(obj, shoes_link, link);
#else
    shoes_link *link;
    Data_Get_Struct(obj, shoes_link, link);
#endif
    link->ele = ele;
    link->start = start;
    link->end = end;
    return obj;
}

VALUE shoes_link_at(shoes_textblock *t, VALUE self, int index, int blockhover, VALUE *clicked, char *touch) {
    char h = 0;
    VALUE url = Qnil;
#ifdef NEW_MACRO_LINK
    Get_TypedStruct2(self, shoes_link, link);
#else
    GET_STRUCT(link, link);
#endif
#ifdef NEW_MACRO_TEXT
    Get_TypedStruct2(link->ele, shoes_text, self_t);
#else
    shoes_text *self_t;
    Data_Get_Struct(link->ele, shoes_text, self_t);
#endif
    if (blockhover && link->start <= index && link->end >= index) {
        h = 1;
        if (clicked != NULL) *clicked = link->ele;
        url = ATTR(self_t->attr, click);
    }

    self = link->ele;
    CHECK_HOVER(self_t, h, touch);
    t->hover = (t->hover & HOVER_CLICK) | h;

    return url;
}

// canvas
VALUE shoes_canvas_link(int argc, VALUE *argv, VALUE self) {
    long i;
    VALUE msgs, attr, text;
    SETUP_CANVAS();
    msgs = rb_ary_new();
    attr = Qnil;
    for (i = 0; i < argc; i++) {
        if (rb_obj_is_kind_of(argv[i], rb_cHash))
            attr = argv[i];
        else
            rb_ary_push(msgs, argv[i]);
    }

    if (rb_block_given_p()) {
        if (NIL_P(attr)) attr = rb_hash_new();
        rb_hash_aset(attr, ID2SYM(s_click), rb_block_proc());
    }

    MARKUP_INLINE(cLink);
    return text;
}
