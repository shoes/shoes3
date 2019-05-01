# yosemite (10.10) build Assumes:
# (1) deps are in Homebrew - /usr/local
# (2) Ruby is from rvm, homebrew or Apples /System, which ever is running Rake
# (3) GEMs follow the ruby path. 
# (4) SDK is the xcode current.
# All of that makes this Shoes non-distributable, probably. Older systems FAIL.
include FileUtils
ignore_deprecations = true
cf =(ENV['ENV_CUSTOM'] || "#{TGT_ARCH}-custom.yaml")
if File.exists? cf
  custmz = YAML.load_file(cf)
  ShoesDeps = custmz['Deps']
  EXT_RUBY = custmz['Ruby'] ? custmz['Ruby'] : RbConfig::CONFIG['prefix']
  APP['GDB'] = 'basic' if custmz['Debug'] == true
  ENV['CDEFS'] = custmz['CFLAGS'] if custmz['CFLAGS']
  APP['GEMLOC'] = custmz['Gemloc'] if custmz['Gemloc']
  APP['EXTLOC'] = custmz['Extloc'] if custmz['Extloc']
  APP['EXTLIST'] = custmz['Exts'] if custmz['Exts']
  APP['GEMLIST'] = custmz['Gems'] if custmz['Gems']
  APP['INCLGEMS'] = custmz['InclGems'] if custmz['InclGems']
  ENV['CDEFS'] = custmz['CFLAGS'] if custmz['CFLAGS']
  ENV['SQLLOC'] = ShoesDeps
  APP['TOOLPRE'] = custmz['ToolPrefix'] ? custmz['ToolPrefix'] : '' #empty string is not nil
  APP['CC'] = custmz['CC'] if custmz['CC']
  APP['RANLIB']= custmz['RANLIB'] if custmz['RANLIB']
  APP['STRIP'] = custmz['STRIP'] if custmz['STRIP']
  OTOOL = APP['TOOLPRE'] + (custmz['OTOOL'] ?  custmz['OTOOL'] : 'otool')
  INTOOL = APP['TOOLPRE'] + (custmz['INTOOL'] ? custmz['INTOOL'] : 'install_name_tool')
  ignore_deprecations = (!custmz['Deprecations']) if custmz['Deprecations']
else
  # create all of the above via the Ruby running this rake step
  EXT_RUBY = RbConfig::CONFIG['prefix']
  if EXT_RUBY.include? '/System/Library' 
    $stderr.puts "We can't use Apple's Ruby. Try rvm or build your own Ruby"
    abort
  end
  ShoesDeps = '/usr/local'
  APP['GDB'] = 'basic'
  APP['CDEFS'] = ENV['CDEFS'] = '-DNEW-RADIO' # TODO still being used?
  APP['TOOLPRE'] = ''
  OTOOL = 'otool'
  INTOOL = 'install_name_tool'
  if EXT_RUBY.include? '.rvm' 
    APP['RVMGEM'] = ENV['GEM_HOME']
    APP['ABSBUNDLE'] = false   # ie. bundles use @executable_path
  elsif EXT_RUBY.include? '/usr/local/Cellar/'  
    # brew install ruby@2.3
    APP['RVMGEM'] = ENV['GEM_HOME']
    APP['ABSBUNDLE'] = true      # ie. bundles use ABS paths
  else
    $stderr.puts "Your ruby is Suspicious. Create a minosx-custom.yaml"
    abort
  end
  
end

# use target ruby
require_relative '../../switch_ruby'

arch_2_file = {
  'x86_64-darwin18' => 'x86_64-darwin18',
  'x86_64-darwin17' => 'x86_64-darwin17',
  'x86_64-darwin14' => 'x86_64-darwin14',
  'x86_64-darwin13' => 'x86_64-darwin13',
}
# Match what Gem:: does (not what you think it should do)
arch_2_gem =  {
  'x86_64-darwin18' => 'x86_64-darwin-18',
  'x86_64-darwin17' => 'x86_64-darwin-17',
  'x86_64-darwin14' => 'x86_64-darwin-14',
  'x86_64-darwin13' => 'x86_64-darwin-13',
}

arch_2_sdk = {'x86_64-darwin18' => 'MacOSX10.14.sdk', # Mojave
              'x86_64-darwin17' => 'MacOSX10.13.sdk', # High Sierra
              'x86_64-darwin16' => 'MacOSX10.12.sdk', # Sierra
              'x86_64-darwin15' => 'MacOSX10.11.sdk', # El Capitan
              'x86_64-darwin14' => 'MacOSX10.10.sdk', # Yosemite
              'x86_64-darwin13' => 'MacOSX10.9.sdk',  # Mavericks
}

SHOES_TGT_ARCH = RbConfig::CONFIG['arch']
SHOES_GEM_ARCH = arch_2_gem[RbConfig::CONFIG['arch']]
#osxsdk = arch_2_sdk[RbConfig::CONFIG['arch']]
osxsdk = 'MacOSX10.12.sdk'  # 12 is as high as we can go w/o code changes
osxminv = osxsdk[/(\d+\.\d+)/]

$stderr.puts "arch #{SHOES_TGT_ARCH} -> #{SHOES_GEM_ARCH}"

APP['RUBY_V'] = RbConfig::CONFIG['ruby_version']
APP['PLATFORM'] = RbConfig::CONFIG['arch'] # now correct for cross compile

CC = APP['TOOLPRE'] + (APP['CC'] ? APP['CC'] : RbConfig::CONFIG["CC"])
RANLIB =APP['TOOLPRE'] + ( APP['RANLIB'] ? APP['RANLIB'] :  RbConfig::CONFIG['RANLIB'])
STRIP = APP['TOOLPRE'] + (APP['STRIP'] ? APP['STRIP'] : RbConfig::CONFIG["STRIP"])
pkgruby ="#{EXT_RUBY}/lib/pkgconfig/#{RbConfig::CONFIG["ruby_pc"]}"

if APP['GDB']
  LINUX_CFLAGS = " -g"
else
  LINUX_CFLAGS = " -O"
end

libdll = "#{ShoesDeps}/lib"
#APP['LIBPATHS'] = [libdll]

# nothing much is going to change for 10.10 deps - don't bother with pkg-config
# because it does go wrong in this situation.
GLIB_CFLAGS   = "-I#{ShoesDeps}/include/glib-2.0 -I#{ShoesDeps}/lib/glib-2.0/include"
GLIB_CFLAGS << " -I#{ShoesDeps}/include/librsvg-2.0/librsvg -I#{ShoesDeps}/include/gdk-pixbuf-2.0/"
GLIB_LDFLAGS  = "-L#{ShoesDeps}/lib -lglib-2.0 -lgobject-2.0 -lintl #{ShoesDeps}/lib/librsvg-2.2.dylib"
GLIB_LDFLAGS << " -lyaml-0.2"
CAIRO_CFLAGS  = "-I#{ShoesDeps}/include/cairo"
CAIRO_LDFLAGS = "-L#{ShoesDeps}/lib -lcairo"
PANGO_CFLAGS  = "-I#{ShoesDeps}/include/pango-1.0"
PANGO_LDFLAGS = "-L#{ShoesDeps}/lib -lpango-1.0"
RUBY_CFLAGS = `pkg-config --cflags "#{pkgruby}"`.strip+" "
#RUBY_LDFLAGS  = `pkg-config --libs "#{pkgruby}"`.strip+" "
#RUBY_LDFLAGS  = "-L#{EXT_RUBY}lib/ -Wl,-undefined,dynamic_lookup -Wl,-multiply_defined,suppress -lruby.2.4.5 -lpthread -ldl -lobjc "

LINUX_CFLAGS << " -I#{ShoesDeps}/include #{GLIB_CFLAGS} #{RUBY_CFLAGS} #{CAIRO_CFLAGS} #{PANGO_CFLAGS}"

LINUX_LIB_NAMES = %W[#{RUBY_SO} cairo pangocairo-1.0 gif]
#LINUX_LIB_NAMES = %W[#{RUBY_LDFLAGS} cairo pangocairo-1.0 gif]

LINUX_CFLAGS << " -DRUBY_1_9 "

DLEXT = "dylib"
#LINUX_CFLAGS << " -DSHOES_QUARTZ -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -fpascal-strings #{RbConfig::CONFIG["CFLAGS"]} -x objective-c -fobjc-exceptions"
LINUX_CFLAGS << " -DVIDEO -DSHOES_QUARTZ -Wall -fpascal-strings -x objective-c -fobjc-exceptions"
LINUX_LDFLAGS = "-framework Cocoa -framework QuartzCore -framework Carbon -dynamiclib -Wl,-single_module INSTALL_NAME"
LINUX_LIB_NAMES << 'pixman-1' << 'jpeg.8'

OSX_SDK = "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/#{osxsdk}"
ENV['MACOSX_DEPLOYMENT_TARGET'] = osxminv
LINUX_CFLAGS << " -mmacosx-version-min=#{osxminv}"
LINUX_LDFLAGS << " -mmacosx-version-min=#{osxminv}"
LINUX_CFLAGS << ' -Wno-incompatible-pointer-types-discards-qualifiers'
if ignore_deprecations
  LINUX_CFLAGS << " -Wno-deprecated-declarations"
end

OSX_ARCH = '-arch x86_64'

ENV['CC'] = CC
ENV['TGT_RUBY_PATH'] = EXT_RUBY
ENV['TGT_ARCH'] = SHOES_TGT_ARCH
ENV['TGT_RUBY_V'] = '2.3.0'  # library version - all 2.3.x rubys
ENV['SYSROOT'] = " -isysroot #{OSX_SDK} #{OSX_ARCH}"

LINUX_CFLAGS << " -isysroot #{OSX_SDK} #{OSX_ARCH}"
LINUX_LDFLAGS << " -isysroot #{OSX_SDK} #{OSX_ARCH} -L#{ShoesDeps}/lib/ #{GLIB_LDFLAGS}"

LINUX_LIBS = " -L #{EXT_RUBY}/lib -l#{RUBY_SO} -L#{ShoesDeps}/lib -l cairo -L#{ShoesDeps}/lib -lpangocairo-1.0 -L#{ShoesDeps}/lib -lgif -ljpeg"
LINUX_LIBS << " -L#{TGT_DIR} #{CAIRO_LDFLAGS} #{PANGO_LDFLAGS} #{GLIB_LDFLAGS}"

# Sigh - shoesdeps and homebrew versions can be out of sync.
# The following dylibs (may) need explicit copying.
SOLOCS = {
  'libcurl.dylib'    => "/usr/lib/libcurl.4.dylib",
  'libxml2.2.dylib' => "/usr/lib/libxml2.2.dylib",
  'libexpat.1.dylib' => "/usr/lib/libexpat.1.dylib",
  'libz.1.dylib'  => "/usr/lib/libz.1.dylib",
  'libgobject-2.0.0.dylib' => "#{libdll}/libgobject-2.0.0.dylib",
  'libgdk_pixbuf-2.0.0.dylib' => "#{libdll}/libgdk_pixbuf-2.0.0.dylib",
  'libgio-2.0.0.dylib' => "#{libdll}/libgio-2.0.0.dylib",
  'libgmodule-2.0.0.dylib' => "#{libdll}/libgmodule-2.0.0.dylib",
  'libcroco-0.6.3.dylib' => "#{libdll}/libcroco-0.6.3.dylib",
}

