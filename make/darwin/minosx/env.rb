# This is for a OSX only build (loose shoes)
# It is safe and desireable to use RbConfig::CONFIG settings
#   Will not build gems or copy gems - use the host ruby.
#   Use deps from Homebrew in /usr/local - 
# This Shoes cannot be distributed - it's not portable.
require 'rbconfig'
ShoesDeps = "/usr/local"  # should not be needed? or used.

APP['GDB'] = "true" # true => compile -g,  don't strip symbols
if APP['GDB']
  LINUX_CFLAGS = "-g -O0"
else
  LINUX_CFLAGS = "-O -Wall"
end

# figure out which ruby we need.
rv =  RUBY_VERSION[/\d.\d/]

LINUX_CFLAGS << " -DRUBY_1_9"
LINUX_CFLAGS << " -DSZBUG"
LINUX_CFLAGS << " -DDEBUG" if ENV['DEBUG']

# Following line may need handcrafting
LINUX_CFLAGS << " -I/usr/include/"

CC = "gcc"

# Query pkg-config for cflags and link settings
EXT_RUBY = RbConfig::CONFIG['prefix']
RUBY_CFLAGS = " #{`pkg-config --cflags #{EXT_RUBY}/lib/pkgconfig/ruby-#{rv}.pc`.strip}"

# Ruby 2.1.2 with RVM has a bug. Workaround or wait for perfection?
rlib = `pkg-config --libs #{EXT_RUBY}/lib/pkgconfig/ruby-#{rv}.pc`.strip
# 2.2.3 is missing  -L'$${ORIGIN}/../lib' in LIBRUBYARG_SHARED in .pc
$stderr.puts "rlib: #{rlib}"
if !rlib[/\-L/]
  #puts "missing -L in #{rlib}" 
  rlib = "-L#{EXT_RUBY}/lib "+rlib
end
if rlib[/{ORIGIN/]
  #abort "Bug found #{rlib}"
  RUBY_LIB = rlib.gsub(/\$\\{ORIGIN\\}/, "#{EXT_RUBY}/lib")
  #RUBY_LIB = rlib
else
  RUBY_LIB = "-L#{EXT_RUBY}/lib " + rlib
end
$stderr.puts "rlib #{rlib}"
SHOES_TGT_ARCH = SHOES_GEM_ARCH =RbConfig::CONFIG['arch'] #'x86_64-darwin14'
CAIRO_CFLAGS = `pkg-config --cflags cairo`.strip
CAIRO_LIB = `pkg-config --libs cairo`.strip
PANGO_CFLAGS = `pkg-config --cflags pango`.strip
PANGO_LIB = `pkg-config --libs pango`.strip
GTK_FLAGS = "#{`pkg-config --cflags gtk+-3.0`.strip}"
GTK_LIB = "#{`pkg-config --libs gtk+-3.0`.strip}"

LINUX_CFLAGS << " -DRUBY_1_9 "

LINUX_CFLAGS << " -DVIDEO -DSHOES_QUARTZ -Wall -fpascal-strings -x objective-c -fobjc-exceptions"
LINUX_LDFLAGS = "-framework Cocoa -framework QuartzCore -framework Carbon -dynamiclib -Wl,-single_module INSTALL_NAME"
#LINUX_LDFLAGS = "-framework Cocoa -framework QuartzCore -framework Carbon -dynamiclib -Wl,-single_module -install-name=#{EXT_RUBY}/lib/libruby.dylib"
LINUX_LIB_NAMES = 'pixman-1 ' << 'jpeg.8'
LINUX_CFLAGS << ' -Wno-incompatible-pointer-types-discards-qualifiers'

OSX_ARCH = '-arch x86_64'

MISC_LIB = " -lgif -ljpeg -lyaml"

# don't use pkg-config for librsvg-2.0 - a warning.
#MISC_CFLAGS = `pkg-config --cflags librsvg-2.0`.strip
MISC_CFLAGS = "-I/usr/local/Cellar/librsvg/2.42.2_2/include/librsvg-2.0/librsvg "
MISC_CFLAGS << "-I/usr/local/Cellar/gdk-pixbuf/2.36.12/include/gdk-pixbuf-2.0 "
MISC_CFLAGS << '-I/usr/local/include '

LINUX_CFLAGS << " #{RUBY_CFLAGS} #{CAIRO_CFLAGS} #{PANGO_CFLAGS} #{MISC_CFLAGS}"

# collect link settings together. Does order matter?
LINUX_LIBS = "#{RUBY_LIB} #{CAIRO_LIB} #{PANGO_LIB} #{MISC_LIB}"
LINUX_LIBS << " -lfontconfig -lpangocairo-1.0 -lpangoft2-1.0" 

# Main Rakefile and tasks.rb needs the below Constants
ADD_DLL = []
DLEXT = "dylib"
SOLOCS = {} # needed to match Rakefile expectations.
=begin
SOLOCS = {
	'libcurl'    => '',
	'libharfbuzz' => '',
	'libiconv' => '',
	'libintl' => '',
	'libjpeg' => '',
	'liblzma' => '',	
	'libpango-1' => '' ,
	'libcairo' => '',	
	'libpangocairo-1' => '',
	'libcroco-0' => '',		
	'libpangoft2-1' => '',		
	'libpixman-1' => '',
	'libexpat' => '',		
	'libpng16' => '',
	'libffi' => '',			
	'librsvg-2' => '',		
	'libfontconfig'	=> '',	
	'libfreetype' => '',		
	'libsqlite3' => '',
	'libgdbm' => '',			
	'libxml2' => '',
	'libgdk_pixbuf-2' => '',	
	'libyaml-0' => '',
	'libgif' => '',		# libGIF.dylib from /sys/lib/frameworks/ImageIO.framework/Versions/A
	'libgio-2' => '',		
	'libglib-2' => '',		
	'libgmodule-2' => '',	
	'libgobject-2' => '',		
	'libgthread-2' => '',
}
=end
=begin
# to save settings 
bld_args = {}
bld_args['CC'] = CC
bld_args['ADD_DLL'] = []
bld_args['DLEXT'] = "so"
bld_args['SOLOCS'] = {}
bld_args['LINUX_CFLAGS'] = LINUX_CFLAGS
bld_args['LINUX_LDFLAGS'] = LINUX_LDFLAGS
bld_args['LINUX_LIBS'] = LINUX_LIBS
File.open("#{TGT_DIR}/build.yaml", 'w') {|f| YAML.dump(bld_args, f)}
=end
