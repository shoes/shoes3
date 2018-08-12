# Build a 32 bit Linux Tight Shoes FOR a pi3 
#   Deps are in usr/lib/, usr/include/ lib/ - copied from Rasbian Stretch
cf =(ENV['ENV_CUSTOM'] || "#{TGT_ARCH}-custom.yaml")
ignore_deprecations = true
if File.exists? cf
  custmz = YAML.load_file(cf)
  ShoesDeps = custmz['Deps']
  EXT_RUBY = custmz['Ruby']
  APP['GDB'] = 'basic' if custmz['Debug'] == true
  APP['GEMLOC'] = custmz['Gemloc'] if custmz['Gemloc']
  APP['EXTLOC'] = custmz['Extloc'] if custmz['Extloc']
  APP['EXTLIST'] = custmz['Exts'] if custmz['Exts']
  APP['GEMLIST'] = custmz['Gems'] if custmz['Gems']
  APP['INCLGEMS'] = custmz['InclGems'] if custmz['InclGems']
  APP['THEME'] = custmz['Theme'] if custmz['Theme']
  ignore_deprecations = (!custmz['Deprecations']) if custmz['Deprecations']
else
  abort "missing #{TGT_ARCH}-custom.yaml"
end

# We don't care about the Ruby running the rake. We want the info
# from the *target* ruby. Ugly or clever hack ahead - you decide
require_relative '../../switch_ruby'

# Ruby doesn't do triplets like everyone else. 
#
arch_2_file = {'arm-linux-eabihf' => 'arm-linux-gnueabihf',
               'arm-linux-gnueabihf' => 'arm-linux-gnueabihf'}
# Match what Gem:: does (not what you think it should do)
arch_2_gem =  {'arm-linux-eabihf' => "arm-linux"}
              
APP['GTK'] = 'gtk+-3.0' # installer needs this to name the output
SHOES_TGT_ARCH = RbConfig::CONFIG['arch']
SHOES_GEM_ARCH = arch_2_gem[RbConfig::CONFIG['arch']]
APP['RUBY_V'] = RbConfig::CONFIG['ruby_version']
APP['PLATFORM'] = RbConfig::CONFIG['arch'] # now correct for cross compile

arch = arch_2_file[RbConfig::CONFIG["arch"]]
uldir = "#{ShoesDeps}/usr/lib"
ularch = "#{ShoesDeps}/usr/lib/#{arch}"
larch = "#{ShoesDeps}/lib/#{arch}"
APP['LIBPATHS'] = [uldir, ularch, larch]
# Cross Compiler and friends 
CC = RbConfig::CONFIG["CC"]
RANLIB = RbConfig::CONFIG['RANLIB']
STRIP = RbConfig::CONFIG["STRIP"] 
# pkg-config setups
pkgconfig = "arm-linux-gnueabihf-pkg-config"
pkg_config_path = "#{ularch}/pkgconfig"
ENV['PKG_CONFIG_PATH'] = pkg_config_path
pkgruby ="#{EXT_RUBY}/lib/pkgconfig/#{RbConfig::CONFIG["ruby_pc"]}"
pkggtk ="#{ularch}/pkgconfig/#{APP['GTK']}.pc" 
# Use Ruby or curl for downloads?
RUBY_HTTP = true
# not everyone provides a librsvg-2.0.pc (or a good one)
rsvg_pc = File.exist? "#{pkg_config_path}/librsvg-2.0.pc"

# Compile flags and includes
if APP['GDB']
  LINUX_CFLAGS = "-g -O0 "
else
  LINUX_CFLAGS = "-O -Wall "
end
LINUX_CFLAGS << "-DRUBY_HTTP -DSHOES_GTK -fPIC "
LINUX_CFLAGS << "-Wno-unused-but-set-variable -Wno-unused-variable "
LINUX_CFLAGS << "-I#{ShoesDeps}/usr/include "
RUBY_CFLAGS =  `#{pkgconfig} --cflags "#{pkgruby}"`.strip+" "
GTK_CFLAGS = `#{pkgconfig} --cflags "#{pkggtk}" --define-variable=prefix="#{ShoesDeps}/usr"`.strip+" "
if false # boo to raspberry librsvg-2.0.pc
  RSVG_CFLAGS = `#{pkgconfig} --cflags librsvg-2.0 --define-variable=prefix="#{ShoesDeps}"`.strip+" "
else
  RSVG_CFLAGS =  "-I#{ShoesDeps}/usr/include/librsvg-2.0/librsvg "
end
tflags = (RUBY_CFLAGS + GTK_CFLAGS + RSVG_CFLAGS).split(' ').uniq
LINUX_CFLAGS << tflags.join(' ');
LINUX_CFLAGS << " -I#{ShoesDeps}/usr/include/#{arch} " 
if ignore_deprecations
  LINUX_CFLAGS << "-Wno-deprecated-declarations "
end

# Link flags and libraries 
CAIRO_LIB = `#{pkgconfig} --libs cairo --define-variable=prefix="#{ShoesDeps}"`.strip
PANGO_LIB = `#{pkgconfig} --libs pango --define-variable=prefix="#{ShoesDeps}"`.strip

if rsvg_pc
  MISC_LIB = `#{pkgconfig} --libs librsvg-2.0 --define-variable=prefix="#{ShoesDeps}"`.strip
else
  MISC_LIB = " #{ularch}/librsvg-2.so"
end
justgif = File.exist? "#{ularch}/libgif.so.7.0.0"
if justgif
  LINUX_LIB_NAMES = %W[gif jpeg yaml]
else
  LINUX_LIB_NAMES = %W[ungif jpeg yaml]
end
DLEXT = "so"
LINUX_LDFLAGS = "-fPIC -shared --sysroot=#{ShoesDeps} -L#{ularch} "
LINUX_LDFLAGS << `#{pkgconfig} --libs #{pkggtk} --define-variable=prefix=#{ShoesDeps}`.strip+" "
# use the ruby link info
RUBY_LDFLAGS = "-rdynamic -Wl,-export-dynamic "
RUBY_LDFLAGS << "-L#{EXT_RUBY}/lib -lruby "
RUBY_LDFLAGS << "-L/usr/#{arch}/lib -lrt -ldl -lcrypt -lm "

LINUX_LIBS = LINUX_LIB_NAMES.map { |x| "-l#{x}" }.join(' ')

LINUX_LIBS << " #{CURL_LDFLAGS if !RUBY_HTTP} #{RUBY_LDFLAGS} #{CAIRO_LIB} #{PANGO_LIB} #{MISC_LIB}"

SOLOCS = {}
SOLOCS['libungif'] = "#{ularch}/libgif.so.7.0.0" if !justgif
SOLOCS['libgif'] = "#{ularch}/libgif.so.7.0.0"  if justgif
SOLOCS['libjpeg'] = "#{ularch}/libjpeg.so.62.2.0"
SOLOCS['libyaml-0'] = "#{ularch}/libyaml-0.so.2.0.5"
SOLOCS['libpcre'] = "#{larch}/libpcre.so.3.13.3"  # needed? 
SOLOCS['libcrypto'] = "#{ularch}/libcrypto.so.1.0.2"
SOLOCS['libssl'] = "#{ularch}/libssl.so.1.0.2"
SOLOCS['libsqlite3'] = "#{ularch}/libsqlite3.so.0.8.6"
SOLOCS['libffi'] = "#{ularch}/libffi.so.6.0.4" 
SOLOCS['librsvg-2'] = "#{ularch}/librsvg-2.so.2.40.16"
SOLOCS['libcurl'] = "#{ularch}/libcurl.so.4.4.0"


# sigh, curl and tyhpoeus - processed in setup.rb

SYMLNK = {}
=begin
SYMLNK['libcurl.so.4.4.0'] = ['libcurl.so', 'libcurl.so.4']
SYMLNK['libgif.so.7.0.0'] = ['libgif.so', 'libgif.so.7']
SYMLNK['libjpeg.so.62.2.0'] = ['libjpeg.so', 'libjpeg.so.62']
SYMLNK['libyaml-0.so.2.0.5'] = ['libyaml.so', 'libyaml-0.so.2']
SYMLNK['libcrypto.so.1.0.2'] = ['libcrypto.so', 'libcrypto.so.1']
SYMLNK['libssl.so.1.0.2'] = ['libssl.so']
SYMLNK['libsqlite3.so.0.8.6'] = ['libsqlite3.so', 'libsqlite3.so.0']
SYMLNK['libffi.so.6.0.4'] = ['libffi.so', 'libffi.so.6']
SYMLNK['librsvg-2.so.2.40.16'] = ['librsvg-2.so', 'librsvg-2.so.2']
=end
