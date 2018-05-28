
module PackShoes
 require 'fileutils'
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
  
  def PackShoes.merge_osx(opts, &blk)
    app_dir = opts['packdir']
    yield "using #{app_dir}" if blk
    # setup defaults if not in the opts
    rbvstr = opts['target_ruby'] ? opts['target_ruby'] : RUBY_VERSION
    rbmm = rbvstr[/\d.\d/].to_str
    # user gems can have a different arch from shoes (hypthen)
    tarch = opts['target_ruby_arch']
    flds = tarch.split('-')
    if flds.size == 2
      rbarch = tarch
      ver = flds[1][/\d\d/]
      puts "parse #{ver.inspect}"
      gemarch = flds[0]+'-darwin-'+ver
      puts "rbarch-1: #{rbarch} garch: #{gemarch}"
    else # assume 3
      gemarch = tarch
      rbarch = "#{flds[0]}-#{flds[1]}#{flds[2]}"
      puts "rbarch-2: #{rbarch} garch: #{gemarch}"
    end
    opts['publisher'] = 'shoerb' unless opts['publisher']
    opts['website'] = 'http://shoesrb.com/' unless opts['website']
    toplevel = []
    Dir.chdir(DIR) do
      Dir.glob('*') {|f| toplevel << f}
    end
    exclude = %w(static CHANGELOG.txt cshoes gmon.out README.txt
      samples package VERSION.txt Shoes.app tmp pangorc command-manual.rb)

    rm_rf "#{app_dir}/#{opts['app_name']}.app"
    mkdir_p "#{app_dir}/#{opts['app_name']}.app/Contents/MacOS"
    mkdir_p "#{app_dir}/#{opts['app_name']}.app/Contents/Resources/English.lproj"
    packdir = "#{app_dir}/#{opts['app_name']}.app/Contents/MacOS"
    # create the resource and sub icons
    app_name = opts['app_name']
    icon_name = File.basename(opts['app_icns'])
    cp opts['app_icns'], "#{app_dir}/#{opts['app_name']}.app/"
    cp opts['app_icns'], "#{app_dir}/#{opts['app_name']}.app/Contents/Resources/"
    vers =[0, 1]
    File.open(File.join(app_dir, "#{opts['app_name']}.app", "Contents", "PkgInfo"), 'w') do |f|
      f << "APPL????"
    end
    File.open(File.join(app_dir, "#{opts['app_name']}.app", "Contents", "Info.plist"), 'w') do |f|
      f << <<END
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleGetInfoString</key>
  <string>#{app_name} #{vers.join(".")}</string>
  <key>CFBundleExecutable</key>
  <string>#{app_name}-launch</string>
  <key>CFBundleIdentifier</key>
  <string>#{opts['osx_identifier']}.#{name}</string>
  <key>CFBundleName</key>
  <string>#{app_name}</string>
  <key>CFBundleIconFile</key>
  <string>#{icon_name}</string>
  <key>CFBundleShortVersionString</key>
  <string>#{vers.join(".")}</string>
  <key>CFBundleInfoDictionaryVersion</key>
  <string>6.0</string>
  <key>CFBundlePackageType</key>
  <string>APPL</string>
  <key>NSHighResolutionCapable</key>
  <string>true</string>
  <key>IFMajorVersion</key>
  <integer>#{vers[0]}</integer>
  <key>IFMinorVersion</key>
  <integer>#{vers[1]}</integer>
</dict>
</plist>
END
    end
  File.open(File.join(app_dir, "#{opts['app_name']}.app", "Contents", "version.plist"), 'w') do |f|
      f << <<END
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>BuildVersion</key>
  <string>1</string>
  <key>CFBundleVersion</key>
  <string>#{vers.join(".")}</string>
  <key>ProjectName</key>
  <string>#{app_name}</string>
  <key>SourceVersion</key>
  <string>#{Time.now.strftime("%Y%m%d")}</string>
</dict>
</plist>
END
    end
    File.open(File.join(packdir, "#{app_name}-launch"), 'wb') do |f|
      f << <<END
#!/bin/bash
APPPATH="${0%/*}"
unset DYLD_LIBRARY_PATH
cd "$APPPATH"
DYLD_LIBRARY_PATH="$APPPATH"  SHOES_RUBY_ARCH="#{opts['target_ruby_arch']}" ./#{app_name}-bin 
END
    end
    chmod 0755, File.join("#{packdir}/#{app_name}-launch")
    yield "Copy Ruby" if blk
    (toplevel-exclude).each do |p|
      cp_r File.join(DIR, p), packdir
    end
    # do the license stuff
    licf = File.open("#{packdir}/COPYING.txt", 'w')
    if opts['license'] && File.exist?(opts['license'])
      IO.foreach(opts['license']) {|ln| licf.puts ln}
      rm_rf "{packdir}/#{File.basename(opts['license'])}"
    end
    IO.foreach("#{DIR}/COPYING.txt") {|ln| licf.puts ln}  
    licf.close
    # we do need some statics for console to work. 
    mkdir_p "#{packdir}/static"
    Dir.glob("#{DIR}/static/icon*.png") {|p| cp p, "#{packdir}/static" }
    if opts['app_png']
      cp "#{opts['app_loc']}/#{opts['app_png']}", "#{packdir}/static/app-icon.png"
    end
=begin
    # remove chipmonk and ftsearch unless requested
    exts = opts['include_exts'] # returns []
    if  !exts || ! exts.include?('ftsearch')
      puts "removing ftsearchrt"
      rm "#{packdir}/lib/ruby/#{rbmm}.0/#{rbarch}/ftsearchrt.bundle" 
      rm_rf "#{packdir}/lib/shoes/help.rb"
      rm_rf "#{packdir}/lib/shoes/search.rb"
    end
    if  !exts || ! exts.include?('chipmunk')
      puts "removing chipmunk"
      rm "#{packdir}/lib/ruby/#{rbmm}.0/#{rbarch}/chipmunk.bundle"
      rm "#{packdir}/lib/shoes/chipmunk.rb"
    end
=end
    # get rid of some things in lib
    rm_rf "#{packdir}/lib/exerb"
    rm_rf "#{packdir}/lib/gtk-2.0" if File.exist? "#{packdir}/lib/gtk-2.0"
    # remove unreachable code in packdir/lib/shoes/ like help, app-package ...
    ['cobbler', 'debugger', 'shoes_irb', 'pack', 'app_package', 'packshoes',
      'remote_debugger', 'winject', 'envgem'].each {|f| rm "#{packdir}/lib/shoes/#{f}.rb" }
  
    # copy app contents (file/dir at a time)
    yield "Copy application" if blk
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
    # remove the Shoes built-in gems if not in the list 
    incl_gems = opts['include_gems']
    rm_gems = []
    Dir.glob("#{packdir}/lib/ruby/gems/#{rbmm}.0/specifications/*gemspec") do |p|
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
      spec = Dir.glob("#{sgpath}/specifications/default/sqlite3*.gemspec")
      rm spec[0]
      rm_gems << File.basename(spec[0],'.gemspec')
    else
      incl_gems.delete("sglite3")
    end
=end
    rm_gems.each do |g|
      puts "Deleting #{g}"
      rm_rf "#{sgpath}/specifications/#{g}.gemspec"
      rm_rf "#{sgpath}/extensions/#{rbarch}/#{rbmm}.0/#{g}"
      rm_rf "#{sgpath}/gems/#{g}"
    end

    # HACK ahead! Copy remaining Shoes gems gem.build_complete files
    # to different arch name because it's needed . Don't know why.
    bld = Dir.glob("#{sgpath}/extensions/#{rbarch}/#{rbmm}.0/*") do |p|
      nm = File.basename(p)
      puts "hack for #{nm}"
      cp_r "#{sgpath}/extensions/#{rbarch}/#{rbmm}.0/#{nm}", 
          "#{sgpath}/extensions/#{gemarch}/#{rbmm}.0"
    end

    # copy requested gems from user's GEMS_DIR - usually ~/.shoes/+gem
    #incl_gems.delete('sqlite3') if incl_gems.include?('sqlite3')
    incl_gems.each do |name| 
      puts "Copy #{name}"
      cp "#{GEMS_DIR}/specifications/#{name}.gemspec", "#{sgpath}/specifications"
      # does the gem have binary?
      built = "#{GEMS_DIR}/extensions/#{gemarch}/#{rbmm}.0/#{name}/gem.build_complete"
      if File.exist? built
        mkdir_p "#{sgpath}/extensions/#{rbarch}/#{rbmm}.0/#{name}"
        cp "#{GEMS_DIR}/extensions/#{gemarch}/#{rbmm}.0/#{name}/gem.build_complete",
          "#{sgpath}/extensions/#{rbarch}/#{rbmm}.0/#{name}"
          
        mkdir_p "#{sgpath}/extensions/#{gemarch}/#{rbmm}.0/#{name}"
        cp "#{GEMS_DIR}/extensions/#{gemarch}/#{rbmm}.0/#{name}/gem.build_complete",
          "#{sgpath}/extensions/#{gemarch}/#{rbmm}.0/#{name}"
        
      end
      cp_r "#{GEMS_DIR}/gems/#{name}", "#{sgpath}/gems"
    end
    
    # hide shoes-bin and shoes launch script names
    yield "make_installer" if blk
    Dir.chdir(packdir) do
      mv 'shoes-bin', "#{opts['app_name']}-bin"
      #chmod 0755, "#{opts['app_name']}"
      rm_rf 'shoes'
      rm_rf 'debug'
      rm_rf 'Shoes.desktop.tmpl'
      rm_rf 'Shoes.remove.desktop'
      rm_rf 'Shoes.remove.tmpl'
      rm_rf 'shoes-install.sh'
      rm_rf 'shoes-uninstall.sh'
      rm_rf 'Shoes.desktop'
      rm_rf 'shoes-launch'
    end
    # now we build scripts that have to be executed manually.
    arch = `uname -m`.strip
    # !!! pkgbuild doesn't work yet!!!
=begin
    # create the plist for pkgbuild to use
    File.open('pkg.plist','w') do |f|
      f << <<END
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<array>
  <dict>
    <key>BundleHasStrictIdentifier</key>
    <true/>
    <key>BundleIsRelocatable</key>
    <true/>
    <key>BundleIsVersionChecked</key>
    <true/>
    <key>BundleOverwriteAction</key>
    <string>upgrade</string>
    <key>RootRelativeBundlePath</key>
    <string>Applications/#{app_dir}</string>
  </dict>
</array>
</plist>
END
    end
    File.open("#{app_dir}/pkg.sh", 'w') do |f|
      f << <<SCR
pkgbuild --root #{app_dir} --identifier #{opts['osx_identifier']}.#{app_name} \\
--component-plist pkg.plist #{app_name}.pkg
SCR
    end
    chmod 0755, "#{app_dir}/pkg.sh"
    # puts "Please examine pkg.sh and then ./pkg.sh to build the .pkg"
    
    # the following works! 
    #`bsdtar -cjf #{app_name}.bz #{app_dir}`
=end
    
    # build a dmg assumes create_dmg is in ./yoursway-create-dmg
    bgp = opts['dmg_background']
    Dir.chdir(app_dir) do
	  File.open("dmg.sh", 'w') do |f|
	    f << <<SCR
#!/bin/bash 
test -f #{app_name}-Installer.dmg && rm #{app_name}-Installer.dmg
#{DIR}/lib/package/yoursway-create-dmg/create-dmg --volname "#{app_name} Installer" \
--window-pos 200 120 \
--window-size 400 300 \
--background #{bgp} \
--icon-size 80 \
--icon #{app_name}.app 150 10 \
--hide-extension #{app_name}.app \
--app-drop-link 150 185 \
--eula #{opts['license']} \
#{app_name}-Installer.dmg #{app_dir}
SCR
	  end
	  chmod 0755, "#{app_dir}/dmg.sh"
	  #puts "For a dmg, do './dmg.sh'"
	  `./dmg.sh`
	  yield "All done!" if blk
	end
  end
end

