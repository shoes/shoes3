 require 'fileutils'
 include FileUtils

module PackShoes
 def PackShoes.rewrite a, before, hsh
    File.open(before) do |b|
      b.each do |line|
        a << line.gsub(/\#\{(\w+)\}/) {
          if hsh[$1] 
            hsh[$1]
          else
            '#{'+$1+'}'
          end
        }
      end
    end
  end
  
  def PackShoes.merge_appimage opts, &blk
    # setup defaults if not in the opts
    #puts opts.inspect
    opts['publisher'] = 'shoerb' unless opts['publisher']
    opts['website'] = 'http://shoesrb.com/' unless opts['website']
    opts['linux_where'] = '/usr/local' unless opts['linux_where']
    lc_name = opts['app_name'].downcase
	  rbmm = RUBY_VERSION[/\d.\d/].to_str
    toplevel = []
    Dir.chdir(DIR) do
      Dir.glob('*') {|f| toplevel << f}
    end
    # local var packdir points to where Shoes is copied - 
    imagedir = "#{opts['packdir']}/#{lc_name}"
    appdir = "#{imagedir}/AppDir"
    packdir = "#{appdir}/usr/lib" 
    rm_rf imagedir
    mkdir_p appdir 
    mkdir_p "#{appdir}/usr/bin"
    mkdir_p "#{appdir}/usr/lib"
    mkdir_p "#{appdir}/usr/share/applications"
    mkdir_p "#{appdir}/usr/share/icons/hicolor/256x256"
    mkdir_p "#{appdir}/usr/share/metainfo"
    # id is very important for appimage and a bit confusing
    arch = 'x86_64'
    if RUBY_PLATFORM =~ /armv/ 
      arch = 'armhf'
    end
    id = "#{opts['rdomain']}.#{lc_name}_#{arch}"
    # copy shoes to packdir
    exclude = %w(static CHANGELOG.txt cshoes.exe gmon.out README.txt
      samples package VERSION.txt tmp debug TODO.txt)
    yield "Copy Shoes" if blk
    symlnks = []
    (toplevel-exclude).each do |p|
      toplvl = File.join(DIR, p)
      if File.symlink? toplvl
        symlnks << [p, File.readlink(toplvl)]
      else
        cp_r toplvl, packdir
      end
    end
    Dir.chdir(packdir) do
      symlnks.each do |pair|
        File.symlink(pair[1], pair[0])
      end
    end
    # do the license stuff
    licf = File.open("#{packdir}/COPYING.txt", 'w')
    IO.foreach("#{DIR}/COPYING.txt") {|ln| licf.puts ln}  
    licf.close
    # we do need some statics for console to work. 
    mkdir_p "#{packdir}/static"
    Dir.glob("#{DIR}/static/icon*.png") {|p| cp p, "#{packdir}/static" }
    if opts['app_png']
      cp "#{opts['app_loc']}/#{opts['app_png']}", "#{packdir}/static/app-icon.png"
    end
    # get rid of some things in lib
    rm_rf "#{packdir}/lib/exerb"
    rm_rf "#{packdir}/lib/gtk-2.0" if File.exist? "#{packdir}/lib/gtk-2.0"
    # remove unreachable code in packdir/lib/shoes/ like help, app-package ...
    ['cobbler', 'debugger', 'shoes_irb', 'pack', 'app_package', 'packshoes',
      'remote_debugger', 'winject', 'envgem'].each {|f| rm "#{packdir}/lib/shoes/#{f}.rb" }
  
    # copy app contents (file/dir at a time)
    yield "Copy #{opts['app_name']}" if blk
    app_contents = Dir.glob("#{opts['app_loc']}/*")
    app_contents.each do |p|
     cp_r p, packdir
    end
    #create new lib/shoes.rb with rewrite
    newf = File.open("#{packdir}/lib/shoes.rb", 'w')
    rewrite newf, "#{DIR}/lib/package/min-shoes.rb", {'APP_START' => opts['app_start'] }
    newf.close
    # create a new lib/shoes/log.rb with rewrite
    logf = File.open("#{packdir}/lib/shoes/log.rb", 'w')
    rewrite logf, "#{DIR}/lib/package/min-log.rb", {'CONSOLE_HDR' => "#{opts['app_name']} Errors"}
    logf.close
    # copy/remove gems - tricksy - pay attention
    # remove the Shoes built-in gems if NOT IN the list 
    yield "Copy gems" if blk
    incl_gems = opts['include_gems'] || Array.new
    rm_gems = []
    gemp = "#{packdir}/lib/ruby/gems/#{rbmm}.0/specifications/*.gemspec"
    #puts "Looking at #{gemp}"
    Dir.glob(gemp) do |p|
      next unless p
      gem = File.basename(p, '.gemspec')
      if incl_gems.include?(gem)
        puts "Keeping Shoes gem: #{gem}"
        incl_gems.delete(gem)
      else
        rm_gems << gem
      end
    end
    sgpath = "#{packdir}/lib/ruby/gems/#{rbmm}.0"
=begin
    # sqlite is a special case so delete it differently - trickery
    if !incl_gems.include?('sqlite3')
      spec = Dir.glob("#{sgpath}/specifications/sqlite3*.gemspec")
      rm spec[0]
      rm_gems << File.basename(spec[0],'.gemspec')
    else
      incl_gems.delete("sglite3")
    end
=end
    rm_gems.each do |g|
      puts "Deleting #{g}"
      rm_rf "#{sgpath}/specifications/#{g}.gemspec"
      rm_rf "#{sgpath}/extensions/#{RUBY_PLATFORM}/#{rbmm}.0/#{g}"
      rm_rf "#{sgpath}/gems/#{g}"
    end
    yield "User gems" if blk
    # copy requested gems from user's Shoes ~/.shoes/+gem/
    # incl_gems will be empty if there are none
    gloc = "#{ENV['HOME']}/.shoes/+gem/"
    incl_gems.each do |name| 
      puts "Copy #{name} from user dir"
      cp "#{gloc}/specifications/#{name}.gemspec", "#{sgpath}/specifications"
      # does the gem have binary?
      built = "#{gloc}/extensions/#{RUBY_PLATFORM}/#{rbmm}.0/#{name}/gem.build_complete"
      if File.exist? built
        mkdir_p "#{sgpath}/extensions/#{RUBY_PLATFORM}/#{rbmm}.0/#{name}"
        cp "#{GEMS_DIR}/extensions/#{RUBY_PLATFORM}/#{rbmm}.0/#{name}/gem.build_complete",
          "#{sgpath}/extensions/#{RUBY_PLATFORM}/#{rbmm}.0/#{name}"
      end
      cp_r "#{gloc}/gems/#{name}", "#{sgpath}/gems"
    end
        
    # hide shoes-bin and shoes launch script names
    yield "Finishing AppImage" if blk
    #after_install = "#{opts['app_name']}_install.sh"
    #before_remove = "#{opts['app_name']}_remove.sh"
    #where = opts['linux_where']
    Dir.chdir(packdir) do
      mv 'shoes-bin', "#{lc_name}-bin"
      File.open("#{lc_name}", 'w') do |f|
        f << <<SCR
#!/usr/bin/env bash
REALPATH=`readlink -f $0`
OLDPWD=`pwd`
APPPATH="${REALPATH%/*}"
if [ "$APPPATH" = "shoes" ]; then
  APPPATH="."
fi
if [ "$APPPATH" = "." ]; then
  APPPATH=`pwd`
fi

# makeself changes the directory to /tmp/selfgzNNNNN.  if we're in
# an extracted archive, change back to the directory where we launched.
if [ `pwd | awk '{ sub(/[0-9]+/, ""); print }'` = "/tmp/selfgz" ]; then
  # fix the symlink, this is actually a bug in the minitar lib
  #rm -f libruby.so.2.1
  #ln -s libruby.so.2.1 libruby.so.2.1.0
  cd "$OLDPWD"
fi
export NO_AT_BRIDGE=1
cd "$OLDPWD"
LD_LIBRARY_PATH=$APPPATH $APPPATH/#{lc_name}-bin
SCR
      end
      chmod 0755, lc_name
    end
    Dir.chdir(appdir) do
      ln_s "usr/lib/#{lc_name}", "AppRun"
    end
    appimgtool = "#{ENV['HOME']}/.shoes/package/appimagetool-#{arch}.AppImage"
    self.make_desktop_appimg appdir, id, opts
    cp "#{appdir}/#{id}.desktop", "#{appdir}/usr/share/applications/"
    cp "#{packdir}/#{opts['app_png']}", "#{appdir}/"
    cp "#{packdir}/#{opts['app_png']}", "#{appdir}/usr/share/icons/hicolor/256x256/"
    
    dest = "#{opts['packdir']}/#{opts['app_name']}.appimage"
    rm_rf dest
    if opts['useXML'] 
      `cp #{opts['useXML']} #{appdir}/usr/share/metainfo/#{id}.appdata.xml`
      result = `#{appimgtool} #{appdir} #{dest}`
      if result && result != ''
        # TODO: check for running from Shoes, if so use alert
        # alert result.inspect
        puts "Error::  #{result}"
      end
    else
    `#{appimgtool} -n #{appdir} #{dest}`
    end
    yield "Done" if blk
  end
  
  def self.make_desktop_appimg(dir, id, opts)
    File.open("#{dir}/#{id}.desktop",'w') do |f|
      f << "[Desktop Entry]\n"
      f << "Name=#{opts['app_name']}\n"
      f << "Exec=#{opts['app_name'].downcase} %f\n"
      f << "StartupNotify=true\n"
      f << "Terminal=false\n"
      f << "Type=Application\n"
      f << "Comment=\"#{opts['purpose']}\"\n"
      f << "Icon=#{File.basename(opts['app_png'], '.png')}\n"
      f << "Categories=#{opts['category']};\n"
      f << "X-AppImage-Integrate=false\n"
    end
  end
end

