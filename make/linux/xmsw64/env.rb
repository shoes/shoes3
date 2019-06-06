# xmsw cross  build  
# TODO: missing curl
# TODO: massage for mxe deps and locations. (basedll..)
cf =(ENV['ENV_CUSTOM'] || "#{APP['VAGRANT']}xmsw64-custom.yaml")
gtk_version = '3'
if File.exists? cf
  custmz = YAML.load_file(cf)
  ShoesDeps = custmz['Deps']
  EXT_RUBY = custmz['Ruby']
  GtkDeps = custmz['GtkLoc'] ? custmz['GtkLoc'] : ShoesDeps
  ENABLE_MS_THEME = custmz['MS-Theme'] == true
  APP['GDB'] = 'basic' if custmz['Debug'] == true
  APP['GEMLOC'] = custmz['Gemloc'] if custmz['Gemloc']
  APP['EXTLOC'] = custmz['Extloc'] if custmz['Extloc']
  APP['EXTLIST'] = custmz['Exts'] if custmz['Exts']
  APP['GEMLIST'] = custmz['Gems'] if custmz['Gems']
  APP['INCLGEMS'] = custmz['InclGems'] if custmz['InclGems']
  APP['VIDEO'] = true
  #APP['GTK'] = 'gtk+-3.0'
  APP['INSTALLER'] = custmz['Installer'] == 'qtifw'? 'qtifw' : 'nsis'
  APP['INSTALLER_LOC'] = custmz['InstallerLoc']
else
  abort "missing #{TGT_ARCH}-custom.yaml"
end

require_relative '../../switch_ruby'

# Ruby doesn't do triplets like everyone else. 
arch_2_file = {'i386-mingw32.shared' => 'i386-mingw32'}
# Match what Gem:: does (not what you think it should do)
arch_2_gem =  {'i386-mingw32.shared' => 'x86-mingw32', # mxe compiled ruby
               'i386-mingw32' => 'x86-mingw32',        # hand compiled ruby
               'x64-mingw32.shared' => 'x64-mingw32'}

SHOES_TGT_ARCH = RbConfig::CONFIG['arch']
SHOES_GEM_ARCH = arch_2_gem[RbConfig::CONFIG['arch']]
APP['RUBY_V'] = RbConfig::CONFIG['ruby_version']
# dll locations for mxe 
bindll = "#{ShoesDeps}/bin"
basedll = "#{ShoesDeps}/basedll"
gtkdll = "#{GtkDeps}/bin"
APP['LIBPATHS'] = [bindll, "#{EXT_RUBY}/bin"]

WINVERSION = "#{APP['VERSION']}-xmsw-64"
WINFNAME = "#{APPNAME}-#{WINVERSION}"

RUBY_HTTP = true

DLEXT = "dll"
# RbConfig doesn't have good values for these:
CC = RbConfig::CONFIG['CC']                 #"i686-w64-mingw32-gcc"
STRIP = "#{RbConfig::CONFIG['STRIP']} -x"   #"i686-w64-mingw32-strip -x"
WINDRES = RbConfig::CONFIG['WINDRES']       #"i686-w64-mingw32-windres"
PKG_CONFIG = "pkg-config"


ENV['PKG_CONFIG_PATH'] = "#{ShoesDeps}/lib/pkgconfig"
#gtk_pkg_path = "#{GtkDeps}/lib/pkgconfig/gtk+-3.0.pc"
pkgruby ="#{EXT_RUBY}/lib/pkgconfig/#{RbConfig::CONFIG["ruby_pc"]}"

WIN32_CFLAGS = []
WIN32_LDFLAGS = []
WIN32_LIBS = []

if APP['GDB']
  WIN32_CFLAGS << "-g3 -O0"
else
  WIN32_CFLAGS << "-O -Wall"
end


#GTK_CFLAGS = `#{PKG_CONFIG} --cflags gtk+-3.0 --define-variable=prefix=#{ShoesDeps}`.chomp
#GTK_LDFLAGS = `#{PKG_CONFIG} --libs #{gtk_pkg_path} --define-variable=prefix=#{ShoesDeps}`.chomp
CAIRO_CFLAGS = `#{PKG_CONFIG} --cflags glib-2.0 --define-variable=prefix=#{ShoesDeps}`.chomp +
    `#{PKG_CONFIG} --cflags cairo --define-variable=prefix=#{ShoesDeps}`.chomp
CAIRO_LDFLAGS = `#{PKG_CONFIG} --libs cairo --define-variable=prefix=#{ShoesDeps}`.chomp
PANGO_CFLAGS = `#{PKG_CONFIG} --cflags pango --define-variable=prefix=#{ShoesDeps}`.chomp
PANGO_LDFLAGS = `#{PKG_CONFIG} --libs pango --define-variable=prefix=#{ShoesDeps}`.chomp
PANGOCAIRO_CFLAGS = `#{PKG_CONFIG} --cflags pangocairo --define-variable=prefix=#{ShoesDeps}`.chomp
PANGOCAIRO_LDFLAGS = `#{PKG_CONFIG} --libs pangocairo --define-variable=prefix=#{ShoesDeps}`.chomp
RSVG_CFLAGS = `#{PKG_CONFIG} --cflags librsvg-2.0 --define-variable=prefix=#{ShoesDeps}`.chomp

#RUBY_LDFLAGS = " -Wl,-export-all-symbols -L#{EXT_RUBY}/lib -lmsvcrt-ruby230 "
RUBY_LDFLAGS = " -Wl,-export-all-symbols -L#{EXT_RUBY}/lib -l#{RbConfig::CONFIG["RUBY_SO_NAME"]} "

WIN32_CFLAGS << "-DSHOES_WIN32 -DRUBY_HTTP -DVIDEO"
WIN32_CFLAGS << "-Wno-unused-but-set-variable -Wno-attributes"
WIN32_CFLAGS << "-D__MINGW_USE_VC2005_COMPAT -DXMD_H -D_WIN32_IE=0x0500 -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -DCOBJMACROS"

WIN32_CFLAGS << CAIRO_CFLAGS
WIN32_CFLAGS << PANGO_CFLAGS
WIN32_CFLAGS << PANGOCAIRO_CFLAGS
WIN32_CFLAGS << RSVG_CFLAGS
WIN32_CFLAGS << "-I#{ShoesDeps}/include/librsvg-2.0/librsvg "
WIN32_CFLAGS << `pkg-config --cflags #{pkgruby} --define-variable=prefix=#{EXT_RUBY}`.chomp
WIN32_CFLAGS << "-Ishoes"

WIN32_LDFLAGS << "-lshell32 -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lcomctl32"
WIN32_LDFLAGS << "-lgif -ljpeg -lfontconfig"
WIN32_LDFLAGS << "-L#{ShoesDeps}/bin"
WIN32_LDFLAGS << "-fPIC -shared"

WIN32_LDFLAGS << CAIRO_LDFLAGS
WIN32_LDFLAGS << PANGO_LDFLAGS
WIN32_LDFLAGS << PANGOCAIRO_LDFLAGS
WIN32_LDFLAGS << RUBY_LDFLAGS

WIN32_LIBS << RUBY_LDFLAGS
WIN32_LIBS << CAIRO_LDFLAGS
WIN32_LIBS << PANGO_LDFLAGS
WIN32_LIBS << PANGOCAIRO_LDFLAGS
WIN32_LIBS << "-L#{ShoesDeps}/lib -lrsvg-2 -lyaml -lpthread"

# Cleaning up duplicates. Clunky? Hell yes!
wIN32_CFLAGS = WIN32_CFLAGS.join(' ').split(' ').uniq
wIN32_LDFLAGS = WIN32_LDFLAGS.join(' ').split(' ').uniq
wIN32_LIBS = WIN32_LIBS.join(' ').split(' ').uniq

LINUX_CFLAGS = wIN32_CFLAGS.join(' ')
LINUX_LDFLAGS = wIN32_LDFLAGS.join(' ')
LINUX_LIBS = wIN32_LIBS.join(' ')


# keys for SOLOCS are globed so libgio libgio-2 libgio-2.0 are the same
# see win_dep_find_and_copy() in Rakefile. Values in the hash are no longer used
SOLOCS = {
  "#{RbConfig::CONFIG["RUBY_SO_NAME"]}"    => "#{EXT_RUBY}/foobar-not-here/msvcrt-ruby230.dll",
  'libgif-7'     => "#{bindll}/libgif-7.dll",
  'libjpeg-9'    => "#{bindll}/libjpeg-9.dll",
  'libyaml-0-2' => "#{bindll}/libyaml-0-2.dll",
  'libiconv-2'   => "#{bindll}/libiconv-2.dll",
  'libgdbm-6'    => "#{bindll}/libgdbm-4.dll",
  'libepoxy-0'   => "#{bindll}/libepoxy-0.dll",  
  'libgcc_s_seh-1' => '',
  'libwebp-7'  => '',
  'libsqlite3-0'  => "#{bindll}/libsqlite3-0.dll",
  'libexpat-1' => "",
  'libbz2' => "",
  'libpcre-1' => "",
  'libtiff-5' => '',
  'liblzma-5' => '',
}


SOLOCS.merge!(
  {
    #'libatk-1.0-0'         => "#{bindll}/libatk-1.0-0.dll",
    'libcairo-2'       => "#{bindll}/libcairo-2.dll",
    'libcairo-gobject-2'  => "#{bindll}/libcairo-gobject-2.dll",
    'libffi-6'         => "#{bindll}/libffi-6.dll",
    'libfontconfig-1'  => "#{bindll}/libfontconfig-1.dll",
    'libfreetype-6'    => "#{bindll}/libfreetype-6.dll",
    'libgdk_pixbuf-2.0-0'   => "#{bindll}/libgdk_pixbuf-2.0-0.dll",
    'libgio-2.0-0'         => "#{bindll}/libgio-2.0-0.dll",
    'libglib-2.0-0'        => "#{bindll}/libglib-2.0-0.dll",
    'libgmodule-2.0-0'     => "#{bindll}/libgmodule-2.0-0.dll",
    'libgobject-2.0-0'     => "#{bindll}/libgobject-2.0-0.dll",
    #'libgdk-3-0'        => "#{gtkdll}/libgdk-3-0.dll", 
    #'libgtk-3-0'        => "#{gtkdll}/libgtk-3-0.dll",
    'libpixman-1-0'      => "#{bindll}/libpixman-1-0.dll", 
    'libintl-8'       => "#{bindll}/libintl-8.dll",
    'libpango-1.0-0'       => "#{bindll}/libpango-1.0-0.dll",
    'libpangocairo-1.0-0'  => "#{bindll}/libpangocairo-1.0-0.dll",
    'libpangoft2-1.0-0'     => "#{bindll}/libpangoft2-1.0-0.dll",
    'libpangowin32-1.0-0'     => "#{bindll}/libpangowin32-1.0-0.dll",
    'libharfbuzz-0'    => "#{bindll}/libharfbuzz-0.dll",
    'libpng16-16'       => "#{bindll}/libpng16-16.dll",
    'libcroco-0.6-3'       => "#{bindll}/libcroco-0.6-3.dll",
    'librsvg-2-2'        => "#{bindll}/librsvg-2-2.dll",
    'libxml2-2'        => "#{bindll}/libxml2-2.dll",
    'libgthread-2.0-0'     => "#{bindll}/libgthread-2.0-0.dll",
    'zlib1'       => "#{bindll}/zlib1.dll",
    'libwinpthread-1'     => "#{basedll}/libwinpthread-1.dll",
  }
)

