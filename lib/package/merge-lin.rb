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
  
  def PackShoes.merge_linux opts, &blk
    # setup defaults if not in the opts
    puts "Starting merge"
    opts['publisher'] = 'shoerb' unless opts['publisher']
    opts['website'] = 'http://shoesrb.com/' unless opts['website']
    opts['linux_where'] = '/usr/local' unless opts['linux_where']
	  rbmm = RUBY_VERSION[/\d.\d/].to_str
    toplevel = []
    Dir.chdir(DIR) do
      Dir.glob('*') {|f| toplevel << f}
    end
    exclude = %w(static CHANGELOG cshoes.exe gmon.out README.md
      samples package VERSION.txt)
    #packdir = "#{opts['app_name']}-app"
    packdir = "#{ENV['HOME']}/.shoes/package/#{opts['app_name']}-app"
    rm_rf packdir
    mkdir_p(packdir) # where fpm will find it.
    # copy shoes
    yield "Copy Shoes" if blk
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
    yield "Setup_installer" if blk
    after_install = "#{opts['app_name']}_install.sh"
    before_remove = "#{opts['app_name']}_remove.sh"
    where = opts['linux_where']
    Dir.chdir(packdir) do
      mv 'shoes-bin', "#{opts['app_name']}-bin"
      File.open("#{opts['app_name']}", 'w') do |f|
        f << <<SCR
#!/bin/bash
REALPATH=`readlink -f $0`
APPPATH="${REALPATH%/*}"
if [ "$APPPATH" = "." ]; then
  APPPATH=`pwd`
fi
LD_LIBRARY_PATH=$APPPATH $APPPATH/#{opts['app_name']}-bin
SCR
      end
      chmod 0755, "#{opts['app_name']}"
      rm_rf 'shoes'
      rm_rf 'debug'
      rm_rf 'Shoes.desktop.tmpl'
      rm_rf 'Shoes.remove.desktop'
      rm_rf 'Shoes.remove.tmpl'
      rm_rf 'shoes-install.sh'
      rm_rf 'shoes-uninstall.sh'
      rm_rf 'Shoes.desktop'
      # still inside packdir. Make an fpm after-install script and some 
      # xdg .desktops
      if opts['create_menu'] == true
        File.open("#{opts['app_name']}.desktop",'w') do |f|
          f << "[Desktop Entry]\n"
          f << "Name=#{opts['app_name']}\n"
          f << "Exec=#{where}/bin/#{opts['app_name']}\n"
          f << "StartupNotify=true\n"
          f << "Terminal=false\n"
          f << "Type=Application\n"
          f << "Comment=#{opts['purpose']}\n"
          f << "Icon=#{where}/lib/#{packdir}/#{opts['app_png']}\n"
          f << "Categories=#{opts['category']};\n"
        end
      end
      File.open(after_install, 'w') do |f|
        f << "#!/bin/bash\n"
        f << "cd #{where}/bin\n"
        f << "ln -s #{where}/lib/#{packdir}/#{opts['app_name']} .\n"
        # do we have a menu?
        if opts['create_menu'] == true
          f << "cd #{where}/lib/#{packdir}\n"
          f << "xdg-desktop-menu install --novendor  #{opts['app_name']}.desktop\n"
        end
      end
      chmod 0755, after_install
      File.open(before_remove, 'w') do |f|
        f << "#!/bin/bash\n"
        f << "rm #{where}/bin/#{opts['app_name']}\n"
        f << "cd #{where}/lib/#{packdir}\n"
        if opts['create_menu'] == true
          f << "xdg-desktop-menu uninstall #{opts['app_name']}.desktop\n"
        end
      end
      chmod 0755, before_remove
    end
    # now we do fpm things - lets build a bash script for debugging
    arch = `uname -m`.strip
    scriptp = "#{ENV['HOME']}/.shoes/package"
    bscr = "#{scriptp}/fpm.sh"
    File.open(bscr,'w') do |f|
      f << <<SCR
#!/bin/bash
echo $PATH
fpm --verbose -t deb -s dir -p #{packdir}.deb -f -n #{opts['app_name']} \
--prefix '#{opts['linux_where']}/lib' --after-install #{packdir}/#{after_install} \
-a #{arch} --url "#{opts['website']}" --license '#{opts['license_tag']}' --before-remove #{packdir}/#{before_remove} \
--vendor '#{opts['publisher']}' --category #{opts['category']} \
--description "#{opts['purpose']}" -m '#{opts['maintainer']}' #{packdir}
SCR
    end
    chmod 0755, bscr
    puts "Please examine fpm.sh and then ./ftm.sh to build the deb"
    #puts `echo $PATH`
    #`#{bscr}`  # no rvm in that shell so no fpm, grrr
    yield "Please do './fpm.sh' in #{scriptp}" if blk
=begin
    # This is troublsome - almost works
    ARGV.clear
    args = ["--verbose", "-t", "deb", "-s", "dir", "-p", "#{packdir}.deb",
     "-f", "-n",  "#{opts['app_name']}", "--prefix", "#{opts['linux_where']}/lib",
     "--after-install", "#{packdir}/#{after_install}", "-a", "#{arch}", "--url",
    "#{opts['website']}", "--license", "\'#{opts['license_tag']}\'", "--before-remove",
    "#{packdir}/#{before_remove}", "--vendor", "'#{opts['publisher']}'", 
     "--category", "#{opts['category']}", "--description", "'#{opts['purpose']}'",
     "-m", "'#{opts['maintainer']}'", "#{packdir}"]
    args.each {|i| ARGV.push i}
    puts ARGV.inspect
    require "rubygems"
    #$: << File.join(File.dirname(__FILE__), "..", "lib")
    puts "Gem path: #{Gem.bin_path('fpm')}"
    require "fpm"
    require "fpm/command"
    FPM::Command.run ARGV
    yield "Your app is in #{scriptp}" if blk
=end
  end
end

