# msys2 native build 
# NOTE this assumes the deps are from an mxe build. 
cf =(ENV['ENV_CUSTOM'] || "msw64-custom.yaml")
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
  APP['GTK'] = 'gtk+-3.0'
  APP['INSTALLER'] = custmz['Installer'] == 'qtifw'? 'qtifw' : 'nsis'
  APP['INSTALLER_LOC'] = custmz['InstallerLoc']
else
  abort "missing #{TGT_ARCH}-custom.yaml"
end
require_relative '../../switch_ruby'

arch_2_gem =  {'i386-mingw32' => 'x86-mingw32',
               'i386-mingw32.shared' => 'x86-mingw32',
               'x64-mingw32'  => 'x64-mingw32'}
SHOES_TGT_ARCH = RbConfig::CONFIG['arch']
SHOES_GEM_ARCH = arch_2_gem[RbConfig::CONFIG['arch']]
APP['RUBY_V'] = RbConfig::CONFIG['ruby_version']

WINVERSION = "#{APP['VERSION']}-gtk3-64"
WINFNAME = "#{APPNAME}-#{WINVERSION}"
WIN32_CFLAGS = []
WIN32_LDFLAGS = []
WIN32_LIBS = []
RUBY_HTTP = true
file_list = []
SRC = FileList[*file_list]
OBJ = SRC.map do |x|
  x.gsub(/\.\w+$/, '.o')
end

DLEXT = "dll"
ADD_DLL = []

CC = "gcc"
ENV['CC'] = CC		# for building sqlite3 gem
ENV['ShoesDeps'] = ShoesDeps # also for sqlite3 gem
STRIP = "strip -s"
WINDRES = "windres"
PKG_CONFIG = "pkg-config.exe"
# dance on ENV['PKG_CONFIG_PATH'] We want something  pkg-config can use
#ENV['PKG_CONFIG_PATH'] = `cygpath -u #{ShoesDeps}/lib/pkgconfig`.chomp 
ENV['PKG_CONFIG_PATH'] = "#{ShoesDeps}/lib/pkgconfig"

if APP['GDB']
  WIN32_CFLAGS << "-g3 -O0"
else
  WIN32_CFLAGS << "-O -Wall"
end


CAIRO_CFLAGS = `#{PKG_CONFIG} --cflags glib-2.0 --define-variable=prefix=#{ShoesDeps}`.chomp +
    `#{PKG_CONFIG} --cflags cairo --define-variable=prefix=#{ShoesDeps}`.chomp
CAIRO_LDFLAGS = `#{PKG_CONFIG} --libs cairo --define-variable=prefix=#{ShoesDeps}`.chomp
PANGO_CFLAGS = `#{PKG_CONFIG} --cflags pango --define-variable=prefix=#{ShoesDeps}`.chomp
PANGO_LDFLAGS = `#{PKG_CONFIG} --libs pango --define-variable=prefix=#{ShoesDeps}`.chomp

RUBY_LDFLAGS = "-L#{RbConfig::CONFIG["bindir"]} #{RbConfig::CONFIG["LIBRUBYARG"]} "
RUBY_LDFLAGS << "-Wl,-export-all-symbols "

WIN32_CFLAGS << "-DRUBY_HTTP -DVIDEO"
WIN32_CFLAGS << "-Wno-unused-but-set-variable -Wno-attributes"
WIN32_CFLAGS << "-D__MINGW_USE_VC2005_COMPAT -DXMD_H -D_WIN32_IE=0x0500 -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -DCOBJMACROS"
WIN32_CFLAGS << CAIRO_CFLAGS
WIN32_CFLAGS << PANGO_CFLAGS
WIN32_CFLAGS << "-I#{ShoesDeps}/include/librsvg-2.0/librsvg "
RbConfig::CONFIG.select { |k, _| k[/hdrdir/] }.each_key do |v|
   WIN32_CFLAGS << "-I#{RbConfig::CONFIG[v]}"
end
WIN32_CFLAGS << "-Ishoes"

WIN32_LDFLAGS << "-lshell32 -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lcomctl32"
WIN32_LDFLAGS << "-lgif -ljpeg -lfontconfig"
# following line probably not needed.
WIN32_LDFLAGS << "-L#{ENV['RI_DEVKIT']}/mingw/bin".gsub('\\','/').gsub(/^\//,'//')
WIN32_LDFLAGS << "-fPIC -shared"
WIN32_LDFLAGS << CAIRO_LDFLAGS
WIN32_LDFLAGS << PANGO_LDFLAGS
WIN32_LDFLAGS << RUBY_LDFLAGS

WIN32_LIBS << RUBY_LDFLAGS
WIN32_LIBS << CAIRO_LDFLAGS
WIN32_LIBS << PANGO_LDFLAGS
WIN32_LIBS << "-L#{ShoesDeps}/lib -lrsvg-2 -lyaml"

# Cleaning up duplicates. Clunky? Hell yes!
wIN32_CFLAGS = WIN32_CFLAGS.join(' ').split(' ').uniq
wIN32_LDFLAGS = WIN32_LDFLAGS.join(' ').split(' ').uniq
wIN32_LIBS = WIN32_LIBS.join(' ').split(' ').uniq

LINUX_CFLAGS = wIN32_CFLAGS.join(' ')
LINUX_LDFLAGS = wIN32_LDFLAGS.join(' ')
LINUX_LIBS = wIN32_LIBS.join(' ')

# hash of dlls to copy in setup.rb
bindll = "#{ShoesDeps}/bin"
basedll = `cygpath -m /mingw32/bin`.chomp
APP['LIBPATHS'] = ["#{EXT_RUBY}/bin/ruby_builtin_dlls", bindll, basedll, "#{EXT_RUBY}/bin"]

# keys for SOLOCS are globed so libgio libgio-2 libgio-2.0 are the same
# see win_dep_find_and_copy() in Rakefile. Values in hash are not used
SOLOCS = {
  "#{RbConfig::CONFIG["RUBY_SO_NAME"]}"    => "#{EXT_RUBY}/foobar-not-here/msvcrt-ruby230.dll",
  'libgif-7'     => "#{bindll}/libgif-7.dll",
  'libjpeg-8'    => "#{bindll}/libjpeg-9.dll",
  'libyaml-0-2' => "#{bindll}/libyaml-0-2.dll",
  'libiconv-2'   => "#{bindll}/libiconv-2.dll",
  'libgdbm-6'    => "#{bindll}/libgdbm-4.dll", 
  'libgdbm_compat-4' => '',
  'libcrypto-1_1-x64' => '',
  'libssl-1_1-x64' => '',
  'libepoxy-0'   => "#{bindll}/libepoxy-0.dll", 
  'libgmp-10'     => "#{basedll}/libgmp-10.dll", # ruby installer likes this
  'libgcc_s_seh-1'  => '', 
  'libsqlite3-0'  => "#{bindll}/libsqlite3-0.dll",
  'libbz2-1'        => '',
  'libpcre-1'     => '',
  'libtiff-5'     => '',
  'liblzma-5'     => '',
  'libfribidi-0' => '',
  'libthai-0' => '',
}

SOLOCS.merge!(
  {
    'libatk-1.0-0'         => "#{bindll}/libatk-1.0-0.dll",
    'libcairo-2'       => "#{bindll}/libcairo-2.dll",
    'libcairo-gobject-2'  => "#{bindll}/libcairo-gobject-2.dll",
    'libffi-6'         => "#{bindll}/libffi-6.dll",
    'libexpat-1'  => '',
    'libfontconfig-1'  => "#{bindll}/libfontconfig-1.dll",
    'libfreetype-6'    => "#{bindll}/libfreetype-6.dll",
    'libgdk_pixbuf-2.0-0'   => "#{bindll}/libgdk_pixbuf-2.0-0.dll",
    'libgio-2.0-0'         => "#{bindll}/libgio-2.0-0.dll",
    'libglib-2.0-0'        => "#{bindll}/libglib-2.0-0.dll",
    'libgmodule-2.0-0'     => "#{bindll}/libgmodule-2.0-0.dll",
    'libgobject-2.0-0'     => "#{bindll}/libgobject-2.0-0.dll",
    'libgdk-3-0'        => "#{gtkdll}/libgdk-3-0.dll", 
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
    'libgraphite2' => '',
    'libstdc++-6' => '',
    'libdatrie-1' => '',
    'libgthread-2.0-0'     => "#{bindll}/libgthread-2.0-0.dll",
    'zlib1'       => "#{bindll}/zlib1.dll",
    'libwinpthread-1'     => "#{basedll}/libwinpthread-1.dll",
  }
)
=begin
SOLOCS = {
  'ruby'    => "#{EXT_RUBY}/bin/msvcrt-ruby230.dll",
  'gif'     => "#{bindll}/libgif-7.dll",
  'jpeg'    => "#{bindll}/libjpeg-8.dll",
  'libyaml' => "#{bindll}/libyaml-0-2.dll",
  'iconv'   => "#{bindll}/libiconv.dll",
  'eay'     => "#{bindll}/libeay32.dll",
  'gdbm'    => "#{bindll}/libgdbm.dll",
  'ssl'     => "#{bindll}/ssleay32.dll",
  'gmp'     => "#{basedll}/libgmp.dll", # ruby 2.2.6+ needs this
  'gcc-dw'  => "#{basedll}/libgcc_s_dw2-1.dll",
  'sqlite'  => "#{bindll}/libsqlite3.dll"
}

if APP['GTK'] == 'gtk+-3.0'
  SOLOCS.merge!(
    {
      'atk'         => "#{bindll}/libatk-1.0.dll",
      'cairo'       => "#{bindll}/libcairo-2.dll",
      'cairo-gobj'  => "#{bindll}/libcairo-gobject-2.dll",
      'ffi'         => "#{bindll}/libffi-6.dll",
      'fontconfig'  => "#{bindll}/libfontconfig-1.dll",
       #'expat'       => "#{bindll}/libexpat-1.dll",
      'freetype'    => "#{bindll}/libfreetype-6.dll",
      'gdkpixbuf'   => "#{bindll}/libgdk_pixbuf-2.0-0.dll",
      'gio'         => "#{bindll}/libgio-2.0-0.dll",
      'glib'        => "#{bindll}/libglib-2.0-0.dll",
      'gmodule'     => "#{bindll}/libgmodule-2.0-0.dll",
      'gobject'     => "#{bindll}/libgobject-2.0-0.dll",
      'gdk3'        => "#{gtkdll}/libgdk-3-0.dll", 
      'gtk3'        => "#{gtkdll}/libgtk-3-0.dll",
      'epoxy'       => "#{bindll}/libepoxy-0.dll",
      'pixman'      => "#{bindll}/libpixman-1-0.dll", 
      'intl8'       => "#{bindll}/libintl-8.dll",
      'pango'       => "#{bindll}/libpango-1.0-0.dll",
      'pangocairo'  => "#{bindll}/libpangocairo-1.0-0.dll",
      'pangoft'     => "#{bindll}/libpangoft2-1.0-0.dll",
      'pango32'     => "#{bindll}/libpangowin32-1.0-0.dll",
      'pixbuf'      => "#{bindll}/libgdk_pixbuf-2.0-0.dll",
      'harfbuzz'    => "#{bindll}/libharfbuzz-0.dll",
      'png16'       => "#{bindll}/libpng16-16.dll",
      'croco'       => "#{bindll}/libcroco-0.6-3.dll",
      'rsvg'        => "#{bindll}/librsvg-2-2.dll",
      'xml2'        => "#{bindll}/libxml2-2.dll",
      'thread'      => "#{bindll}/libgthread-2.0-0.dll",
      'zlib1'       => "#{bindll}/zlib1.dll",
      #'pthread'     => "#{bindll}/libwinpthread-1.dll",
      'pthread'     => "#{basedll}/libwinpthread-1.dll",
      #'sjlj'        => "#{bindll}/libgcc_s_sjlj-1.dll" 
    }
  )
end
=end
