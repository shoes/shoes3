# This is for a freebsd only build (loose shoes)
# it uses symlinks so it Cannot be distributed.
# This MAY depend on rvm
#   Will not build gems or copy gems - uses the host ruby and gems.
#   Set your $PATH to the ruby you want to use (Install RVM? or similar)
require 'rbconfig'
ignore_deprecations = false # user choice

APP['GDB'] = "true" # true => compile -g,  don't strip symbols
if APP['GDB']
  LINUX_CFLAGS = "-g -O0"
else
  LINUX_CFLAGS = "-O -Wall"
end

# figure out which ruby we need.
rv =  RUBY_VERSION[/\d.\d/]

LINUX_CFLAGS << " -DRUBY_HTTP -DBSD"
LINUX_CFLAGS << " -DRUBY_1_9"
LINUX_CFLAGS << " -DDEBUG" if ENV['DEBUG']
LINUX_CFLAGS << " -DSHOES_GTK -fPIC -DSZBUG -DGTK_CANVAS_SIZE"
# Following line may need handcrafting
LINUX_CFLAGS << " -I/usr/include/"
LINUX_CFLAGS << " #{`pkgconf --cflags gtk+-3.0`.strip}"

CC = "cc"

# Query pkgconf for cflags and link settings
EXT_RUBY = RbConfig::CONFIG['prefix']
RUBY_CFLAGS = " #{`pkgconf --cflags #{EXT_RUBY}/lib/pkgconfig/ruby-#{rv}.pc`.strip}"
rlib = `pkgconf --libs #{EXT_RUBY}/lib/pkgconfig/ruby-#{rv}.pc`.strip
# need to rewrite rlib for an RVM installed Ruby - rvm or Ruby bug?
$stderr.puts "EXT_RUBY: #{EXT_RUBY}"
if EXT_RUBY =~ /\.rvm/
  RUBY_LIB = "-L#{EXT_RUBY}/lib -Wl,-rpath,#{EXT_RUBY}/lib -lruby -lunwind -lexecinfo -lprocstat -lthr -lgmp -lcrypt -lm"
else
  RUBY_LIB = rlib
end

CAIRO_CFLAGS = `pkgconf --cflags cairo`.strip
CAIRO_LIB = `pkgconf --libs cairo`.strip
PANGO_CFLAGS = `pkgconf --cflags pango`.strip
PANGO_LIB = `pkgconf --libs pango`.strip
GTK_FLAGS = "#{`pkgconf --cflags gtk+-3.0`.strip}"
GTK_LIB = "#{`pkgconf --libs gtk+-3.0`.strip}"

MISC_LIB = " -lgif -ljpeg -lyaml"

# don't use pkg-config for librsvg-2.0 - a warning.
MISC_CFLAGS = ' '

MISC_CFLAGS << "-I/usr/local/include/librsvg-2.0/librsvg "
MISC_LIB << " /usr/local/lib/librsvg-2.so"

# collect flags together
LINUX_CFLAGS << " #{RUBY_CFLAGS} #{GTK_FLAGS} #{CAIRO_CFLAGS} #{PANGO_CFLAGS} #{MISC_CFLAGS}"
if ignore_deprecations
  LINUX_CFLAGS << " -Wno-deprecated-declarations"
end
# collect link settings together
LINUX_LIBS = "#{RUBY_LIB} #{GTK_LIB}  #{CAIRO_LIB} #{PANGO_LIB} #{MISC_LIB}"
LINUX_LIBS << " -lfontconfig"
# the following is only used to link the shoes code with main.o - not needed?
#LINUX_LDFLAGS = "-L. -rdynamic -Wl,-export-dynamic"

# Main Rakefile and tasks.rb needs the below Constants
ADD_DLL = []
DLEXT = "so"
SOLOCS = {} # needed to match Rakefile expectations.
