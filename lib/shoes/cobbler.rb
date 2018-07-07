# Cobbler - various Shoes maintenance things
# CJC: the gem handling is kind of ugly. Someone should make it pretty.
require 'rubygems'
require 'rubygems/dependency_installer'
require 'rubygems/uninstaller'
require 'rubygems/package'

module Gem
  if Shoes::RELEASE_TYPE =~ /TIGHT/
    @ruby = (File.join(RbConfig::CONFIG['bindir'], 'shoes') + RbConfig::CONFIG['EXEEXT']).
          sub(/.*\s.*/m, '"\&"') + " --ruby"
  end
end
class << Gem::Ext::ExtConfBuilder
  alias_method :make__, :make
  def make(dest_path, results)
    raise unless File.exist?('Makefile')
    mf = File.read('Makefile')
    mf = mf.gsub(/^INSTALL\s*=\s*.*$/, "INSTALL = $(RUBY) -run -e install -- -vp")
    mf = mf.gsub(/^INSTALL_PROG\s*=\s*.*$/, "INSTALL_PROG = $(INSTALL) -m 0755")
    mf = mf.gsub(/^INSTALL_DATA\s*=\s*.*$/, "INSTALL_DATA = $(INSTALL) -m 0644")
    File.open('Makefile', 'wb') {|f| f.print mf}
    make__(dest_path, results)
  end
end

class Gem::CobblerFace
  class DownloadReporter  #ProgressReporter
    attr_reader :count

    def initialize(prog, status, size, initial_message,
                   terminal_message = "complete")
      @prog = prog
      (@status = status).replace initial_message
      @total = size
      @count = 0.0
    end

    def updated(message)
      @count += 1.0
      @prog.fraction = (@count / @total.to_f) * 0.5
    end

    def fetch(filename, len)
      @total = len
    end

    def update(len)
      @prog.fraction = len.to_f / @total.to_f
    end

    def done
    end
  end

  # init CobblerFace
  def initialize bar, statline
    @prog = bar
    @status = statline
    #@status, @prog, = app.slot.contents[-1].contents

  end
  def title msg
    @status.text =  msg
  end
  def progress count, total
    @prog.fraction = count.to_f / total.to_f
    #$fraction = count.to_f / total.to_f
  end
  def ask_yes_no msg
    Kernel.confirm(msg)
  end
  def ask msg
    Kernel.ask(msg)
  end

  def error msg, e
    @status =  link("Error") { Shoes.show_log }, " ", msg
  end

  def say msg
    @status.text =  msg
  end

  def alert msg, quiz=nil
    ask(quiz) if quiz
  end

  def download_reporter(*args)
    DownloadReporter.new(@prog, @status, 0, 'Downloading')
  end

  def method_missing(*args)
    p args
    nil
  end
end

# UI class for delete does nothing - on purpose. Swallows 'success' msg
class Gem::CobblerDelFace
 def initialize
 end
 def say msg
   #puts "CmdFace: say: #{msg}"
 end
end

Shoes.app :title => "Shoes Cobbler", menus: true do
  @shoes_home = File.join(LIB_DIR, Shoes::RELEASE_NAME)
  stack do
    @mb = menubar
    @mb[0].remove "Cobbler"
    @mb[0].remove "Package"
    @helpmenu = menu "Help"
    infoitem = menuitem "Info", key: "control_i" do
      infoscreen
    end
    splashitem = menuitem "Splash" do
      splash_screen
    end
    @helpmenu << infoitem
    @helpmenu << splashitem
    @gemmenu = menu "Gems"
    mgemitem = menuitem "Manage", key: "control_g" do
      gemscreen
    end
    @gemmenu << mgemitem
    if Shoes::RELEASE_TYPE =~ /TIGHT/
      jbitem = menuitem "Jail Break" do
          jailscreen
      end
      @gemmenu << jbitem
    end
    gpitem = menuitem "Install Gempack" do
      gempack_screen
    end
    @gemmenu << gpitem
    @pfmenu = menu "Setup"
    cpitem = menuitem "Copy Samples" do
      cp_samples_screen
    end
    @pfmenu << cpitem
    pkitem = menuitem "Package URLs" do
      pack_screen
    end
    @pfmenu << pkitem
    vlcitem = menuitem "VLC setup" do
      vlc_screen
    end
    @pfmenu << vlcitem
    ccitem = menuitem "Clear Cache" do
      cachescreen
    end
    @pfmenu << ccitem
    if RUBY_PLATFORM =~ /darwin/
      csitem = menuitem "Setup ./cshoes" do
        cshoes_screen
      end
      @pfmenu << csitem
    end
    @mb << @gemmenu
    @mb << @pfmenu
    if RUBY_PLATFORM =~ /linux|bsd|mingw/
      @thememenu = menu "Themes"
      switem = menuitem "Switch theme" do
        swtheme_screen
      end
      @thememenu << switem
      britem = menuitem "Browse Online"
      britem.enable = false
      @thememenu << britem
      @mb << @thememenu
    end
    @pkgmenu = menu "Package"
    shyitem = menuitem "Shy archive"
    @pkgmenu << shyitem
    xpitem = menuitem "Cross Platform" do
      Shoes.app_package & close
    end
    @pkgmenu << xpitem
    exeitem = menuitem "Windows Merge" do
      win_merge_screen
    end
    exeitem.enable = false unless RUBY_PLATFORM =~ /mingw/
    @pkgmenu << exeitem
    osxitem = menuitem "Advanced OSX" do
      osx_merge_screen
    end
    osxitem.enable = false unless RUBY_PLATFORM =~ /darwin/
    @pkgmenu << osxitem
    debitem = menuitem "Linux Merge .deb" do
      deb_merge_screen
    end
    debitem.enable = false unless RUBY_PLATFORM =~ /linux/
    @pkgmenu << debitem
    bsditem = menuitem "Freebsd Merge" do
      bsd_merge_screen
    end
    bsditem.enable = false unless RUBY_PLATFORM =~ /linux|bsd/
    @pkgmenu << bsditem
    @mb << @pkgmenu
    @mb << @helpmenu
=begin    
    @menu = flow do
      button "Shoes Info" do
        infoscreen
      end
      button "Clear Image Cache" do
        cachescreen
      end
      if Shoes::RELEASE_TYPE =~ /TIGHT/
        button "Jail Break Gems" do
          jailscreen
        end
      end
      button "Manage Gems" do
        gemscreen
      end
      if Shoes::RELEASE_TYPE =~ /TIGHT/ || true # for testing.
        button "Install Gempack" do
          gempack_screen
        end
      end
      button "Profile" do
        require 'shoes/profiler'
        Shoes.profile(nil)
      end
      button "Copy Samples" do
        cp_samples_screen
      end
      button "Packager URLs" do
        pack_screen
      end
      if RUBY_PLATFORM =~ /darwin/
        button "cshoes" do
          cshoes_screen
        end
      end
      button "VLC setup" do
        vlc_screen
      end
      button "Manual" do
        Shoes.show_manual
      end
      button "Splash" do
        Shoes.splash
      end
      button "Quit" do
         Shoes.quit
      end
    end
=end
    @panel = stack do
      @status = para ""
   end
   start { splash_screen }
  end

  def  pack_screen
    @panel.clear
    @panel.append do
      flow do
        para "CGI selector  "
        sel = edit_line "http://shoes.mvmanila.com/public/select/pkg.rb", width: 400
        button "Update" do
          mkdir_p "#{@shoes_home}/package"
          File.open("#{@shoes_home}/package/selector.url", 'w') do |f|
            f.write sel.text
          end
        end
      end
      flow do
        para "Download  "
        dnl = edit_line "http://shoes.mvmanila.com/public/shoes", width: 400
        button "Update" do
          mkdir_p "#{@shoes_home}/package"
            File.open("#{@shoes_home}/package/download.url", 'w') do |f|
            f.write dnl.text
          end
        end
      end
    end
  end
  
  def vlc_screen
    require_relative 'vlcpath'
    Vlc_path.load File.join(LIB_DIR, Shoes::RELEASE_NAME, 'vlc.yaml')
    @panel.clear
    @panel.append do
      para "Set paths for your VLC Installation. Pick the libvlc.dll for \
the first selection and then the Folder named plugins"
      flow do
        para "libvlc.dll [.so, .dylib]"
        @vlcapp = edit_line "#{ENV['VLC_APP_PATH']}", width: 320
        button "Update" do
          vlcp = ask_open_file
          if vlcp
            @vlcapp.text = vlcp
          end
        end
      end
      flow do
        para "VLC Plugin Path "
        @vlcplug = edit_line "#{ENV['VLC_PLUGIN_PATH']}", width: 350
        button "Update" do
          vlpp = ask_open_folder
          if vlpp
            @vlcplug.text = vlpp
          end
       end
     end
      flow do
        button "Save" do
          if ! (Vlc_path.check @vlcapp.text, @vlcplug.text)
            alert "Those are not good paths. Try again."
          else 
            ENV['VLC_APP_PATH'] = @vlcapp.text
            ENV['VLC_PLUGIN_PATH'] = @vlcplug.text
            mkdir_p File.join(LIB_DIR, Shoes::RELEASE_NAME)
            Vlc_path.save File.join(LIB_DIR, Shoes::RELEASE_NAME, 'vlc.yaml')
          end
        end
        button "Remove" do
          ENV['VLC_APP_PATH'] =  @vlcapp.text = ''
          ENV['VLC_PLUGIN_PATH'] = @vlcplug.text = ''
          rm File.join(LIB_DIR, Shoes::RELEASE_NAME, 'vlc.yaml')
        end
      end
    end
  end

  def infoscreen
    @panel.clear
    @panel.append do
      para "Ruby Version: #{RUBY_VERSION} on #{RUBY_PLATFORM}"
      para "Shoes Release: #{Shoes::RELEASE_NAME}   #{Shoes::VERSION_NUMBER}  r#{Shoes::VERSION_REVISION}"
      para "    built on #{Shoes::VERSION_DATE}"
      para "    Fit: #{Shoes::RELEASE_TYPE}"
      para "Gems Version #{Gem::RubyGemsVersion}"
      para "Shoes Exe Directory: #{DIR}"
      para "Shoes Home: #{@shoes_home}"
      para "LIB_DIR: #{LIB_DIR}"
    end
  end

  def jailscreen
    @panel.clear
    @panel.append do
      para "Use directories below for loading gems"
      dirlist = []
      #jloc = "#{ENV['HOME']}/.shoes/#{Shoes::RELEASE_NAME}/getoutofjail.card"
      jloc = File.join(LIB_DIR, Shoes::RELEASE_NAME, 'getoutofjail.card')
      if File.exist? jloc
        open(jloc, 'r') do |f|
         f.each do |l|
           ln = l.strip
           dirlist << ln if ln.length > 0
         end
        end
      end
      @dirbox = edit_box :width => 600
      @dirbox.text = dirlist.join("\n") if dirlist.length > 0
      flow do
	      button "Add Gem Directory" do
	        dir = ask_open_folder
	        dirlist << dir if dir
	        @dirbox.text = dirlist.join("\n")
	      end
	      button "Save JailBreak Values" do
	        dirlist = @dirbox.text.split("\n")
	        open(jloc,'w') do |f|
	          dirlist.each {|ln| f.puts ln}
	        end
	        @panel.append {para "Please quit and restart Shoes for changes to take effect"}
	      end
	  end

    end
  end

  def imgdeletef
  end

  def cp_samples_screen
    @panel.clear
    @panel.append do
      para "Copy samples to a directory you can see and edit."
      para "Chose a directory that you want the samples directories"
      para "to be created inside of."
      button "Select Directory for a copy to" do
        # OSX is a bit brain dead for ask_save_folder
        if destdir = ask_save_folder()
          @panel.append do
            para "Copy #{DIR}/samples/* to #{destdir} ?"
            button "OK" do
              @panel.append do
                @lb = edit_box width: 400
              end
              ary = []
              require 'fileutils'
              mkdir_p destdir
              sampdir = File.join DIR, 'samples'
              ary.push "In #{destdir}"
              cd sampdir do
                Dir.glob('*') do |d|   # simple, good, expert in a perfect world
                  mkdir_p "#{destdir}/#{d}"
                  Dir.glob("#{d}/*").each do |f|
                    cp_r f, "#{destdir}/#{d}"
                    ary << f
                  end
                end
                @lb.text = ary.join("\n")
              end
              #alert "copied"
             end
           end
         end
       end
    end
  end

  def cachescreen
    @panel.clear
    require 'shoes/data'
    require 'shoes/image'
    litems = ['-all-']
    @panel.append do
      @cflow = flow do
        button "Delete Image Cache" do
          app.cache_clear :all if @cache_all.checked? 
          app.cache_clear :memory if @cache_int.checked? 
          app.cache_clear :external if @cache_ext.checked?
          quit if confirm "You should restart Shoes"
        end
        @cache_all = radio :imgcache; para "Both caches"
        @cache_int = radio :imgcache ; para "Internal images"
        @cache_ext = radio :imgcache; para "External images"
        @cache_all.checked = true
      end
    end
  end

  def gem_reset
    Gem.use_paths(GEM_DIR, [GEM_DIR, GEM_CENTRAL_DIR])
    Gem.refresh
  end


  def gem_install_one spec
    # setup Gem download ui
    ui = Gem::DefaultUserInteraction.ui = Gem::CobblerFace.new(@progbar, @status)
    ui.title "Installing #{spec.name} #{spec.version}"
    installer = Gem::DependencyInstaller.new
    begin
      installer.install(spec.name, spec.version)
      gem_reset
      spec.activate
      ui.say "Finished installing #{spec.name}"
    rescue Object => e
       puts "Fail #{e}"
       @status.replace link("Error") { Shoes.show_log }, " ", e
       #ui.error "while installing #{spec.name}", e
       #raise e
    end
  end

  def geminfo gem
    str = gem
    if gem.kind_of? Gem::Specification
      str = "#{gem.name} #{gem.version} #{gem.summary}\nSee: #{gem.homepage}"
    end
    alert str
  end

  def gemremove spec
    if confirm "Really delete gem #{spec.name}"
      begin
        Gem::DefaultUserInteraction.ui = Gem::CobblerDelFace.new()
        del = Gem::Uninstaller.new(spec)
        del.remove(spec)
      rescue Exception => e
        alert e
      end
      gem_refresh_local
    end
  end

  def geminstall spec
   @gemlist.append do
     @progbar = progress width: 0.9, margin_left: 0.1, height: 20
     @progbar.fraction = 0.5
     @status = inscription 'Initialize', align: :center
   end
   if confirm "Install #{spec.name},#{spec.version} and dependencies?"
      @thread = Thread.new do
        gem_install_one spec
        #gem_refresh_local  # not sure I want this UI wise.
      end
    end
  end

  def gemloadtest spec
    begin
      require spec.name
    rescue Exception => e
      alert e
    end
    alert "Loaded"
  end


  def gem_refresh_local
    @gemlist.clear
    @gemlist.background white
    # FIXME: deprecated call, returns []
    #gemlist =  Gem::Specification._all()
    #gemlist.each do |gs|
    Gem::Specification.each do |gs|
      @gemlist.append do
      flow margin: 5 do
          button 'info', height: 28, width: 50, left_margin: 10 do
             geminfo gs
           end
           button 'delete', height: 28, width: 60, left_margin: 10 do
             gemremove gs
           end
           button 'load test', height: 28, width: 60, left_margin: 10 do
             gemloadtest gs
           end
           para "#{gs.name}, #{gs.version}"
         end
       end
    end
  end

  def gemsearch str
    installer = Gem::DependencyInstaller.new domain: :remote
    begin
      poss_gems = installer.find_spec_by_name_and_version(str, Gem::Requirement.default)
    rescue Gem::SpecificGemNotFoundException => e
      @gemlist.append {para "not found"}
      return
    end
    poss_gems.each_spec do |g|
      @gemlist.append do
        flow margin: 5 do
          button 'info', height: 28, width: 50, left_margin: 10 do
            geminfo g
          end
          button 'install', height: 28, width: 62, left_margin: 10 do
            geminstall g
          end
          para "#{g.name},#{g.version}"
        end
      end
    end
  end

  def gemscreen
    @panel.clear
    @panel.append do
      para "Manage Gems"
      flow do
        button 'Show Local' do
          @gemlist.clear
          @gemlist.background white
          gem_refresh_local
        end
        @searchphrase = edit_line
        button 'Search Remote' do
          @gemlist.clear
          @gemlist.background "#EEE".."#9AA"
          gemsearch @searchphrase.text
        end
      end
      @gemlist = stack width: 0.90, left_margin: 5, top_margin: 5 do
        background white
      end
    end
  end

  def tar_extract opened_file
    Gem::Package::TarReader.new( Zlib::GzipReader.new(opened_file)) do |tar|
      tar.each do |entry|
        dest = entry.full_name
	    if entry.directory?
	      FileUtils.rm_rf dest unless File.directory? dest
	      FileUtils.mkdir_p dest, :mode => entry.header.mode, :verbose => false
	    elsif entry.file?
	      FileUtils.rm_rf dest unless File.file? dest
	      File.open dest, "wb" do |f|
	        f.print entry.read
	       end
	       FileUtils.chmod entry.header.mode, dest, :verbose => false
	    elsif entry.header.typeflag == '2' #Symlink!
	      alert "Cannot convert Symlinks. Contact #{hdr.creator}"
	    end
      end
    end
  end

  def gem_copy_to_home srcdir, dest
    gems = Dir.glob("#{srcdir}/*/*")
    mkdir_p dest
    #return if !confirm "#{gems} from #{srcdir} to #{dest}"
    gems.each do |gempath|
      # look inside for the gem.build_complete
      gemn = File.split(gempath)[1]
      if File.exists? File.join(gempath,'gem.build_complete')
        extpath = File.join(dest, 'extensions', "#{Gem::Platform.local}", '2.1.0', gemn)
        puts extpath
        mkdir_p extpath
        cp File.join(gempath,'gem.build_complete'), extpath
      end
      # copy the gemspec
      specpath = File.join(dest, 'specifications')
      mkdir_p specpath
      specname = gemn+'.gemspec'
      cp File.join(gempath,'gemspec'), File.join(specpath, specname)
      # copy ext if we have one
      if File.exists? File.join(gempath, 'ext')
        puts "Copy ext #{gempath}"
        mkdir_p File.join(dest, 'gems', gemn)
        cp_r File.join(gempath, 'ext'), File.join(dest,'gems', gemn)
      end
      # copy lib if we have it
      if File.exists? File.join(gempath, 'lib')
        mkdir_p File.join(dest, 'gems', gemn)
        cp_r File.join(gempath, 'lib'), File.join(dest,'gems', gemn)
      end
    end
    gemlist = []
    gems.each {|g| gemlist << File.basename(g) }
    return gemlist
  end

  def gempack_helper tgzpath
    # make a temp directory and unpack the tgzpath into it
    # loop thru the 'special' gems in there and copy into GEM_DIR
    td = Dir.mktmpdir('gempack')
    tarf = File.open(tgzpath,'rb')
    Dir.chdir(td) do |d|
      tar_extract tarf # if confirm "Copy #{tgzpath} to #{GEM_DIR} via #{td}"
    end
    #just begining to get ugly -FIXME -- need th
    return gem_copy_to_home td, GEM_DIR
  end

  def gempack_screen
    @panel.clear
    @panel.append do
      para "Load binary gems from a tgz file. Be Careful! - You can break Shoes \
with the wrong package for your plaftorm!"
      button "Select file..." do
        gempack = ask_open_file
        if gempack
          gemslist = gempack_helper gempack
          gemslist.each do |g|
            para "Installed #{g}\n"
          end
          para "---------------------------\n"
          para "You must quit and restart Shoes to see these gems"
        end
      end
    end
   end

   # not that pretty at all.
   def rewrite_with_env before, after, reg = /\#\{(\w+)\}/, reg2 = '\1'
     File.open(after, 'w') do |a|
       File.open(before) do |b|
         b.each do |line|
           a << line.gsub(reg) do
             if reg2.include? '\1'
               reg2.gsub(%r!\\1!, ENV[$1])
             else
               reg2
             end
           end
         end
       end
     end
   end
   
  def splash_screen
    @panel.clear do
      stack width: 598, height: 520 do 
        background "#{DIR}/static/splash.png"
        para 'Welcome to', align: 'center', weight: 'bold', stroke: ivory, size: 18, margin: 0
        para 'Cobbler', align: 'center', weight: 'bold', size: 24, stroke: ivory, margin: 0
        para Shoes::VERSION_NAME, align: 'center', stroke: ivory, size: 14, margin: 0, weight: 'bold'
        para "build #{Shoes::VERSION_NUMBER} r#{Shoes::VERSION_REVISION}", align: 'center', weight: 'bold', size: 14, stroke: ivory, margin_top: 0
      end 
    end
  end


  def cshoes_screen
    @panel.clear
    @panel.append do
      stack do
          para "Create a cshoes shell script for terminal users to use. \
OSX has to use absolute paths for launching and cshoes script tries to compensate \
but it needs to know where Shoes is"
          para "Select the Shoes.app to point to. "
          this_shoes = "/Applications/Shoes.app"
          flow do
            @shoes_loc = edit_line this_shoes, :width => 300
            button "Select..." do
              this_shoes = ask_open_file
              if this_shoes
                @shoes_loc.text = this_shoes
              end
            end
          end
          new_loc = "#{ENV['HOME']}"
          flow do
            para "Save here:  "
            @cshoes_loc = edit_line new_loc
            button "Select ..." do
              new_loc = ask_save_folder
              if new_loc
                @cshoes_loc.text = new_loc
              end
            end
          end
          flow do
            para "Ready? "
            button "Create Script" do
              if confirm "Create #{@cshoes_loc.text}/cshoes Using #{@shoes_loc.text}"
                #prepare for rewrite - expect complaints about constants
                ENV['SHOES_RUBY_ARCH'] = RUBY_PLATFORM
                ENV['APPNAME'] = 'Shoes'
                ENV['TGT_DIR'] = @shoes_loc.text
                rewrite_with_env  File.join(DIR,"static/stubs/cshoes.templ"), "#{@cshoes_loc.text}/cshoes"
                chmod 0755, "#{@cshoes_loc.text}/cshoes"
              end
            end
          end
        end
      end
    end
    
  def swtheme_screen
    @panel.clear
    @panel.append do
      stack do
        tagline "Change the Theme."
        if RUBY_PLATFORM =~ /mingw/
          path = "#{ENV['HOME']}/AppData/Local/Shoes/themes"
        else
          path = "#{ENV['HOME']}/.shoes/themes"
        end
        para "Searching in #{path}"
        items = []
        mkdir_p path
        Dir.chdir(path) do
          items = Dir.glob("*")
        end
        sel = nil
        if File.exist? "#{path}/default"
          items -= ["default"]
          f = File.new("#{path}/default", 'r') 
          sel = f.readline.strip
          f.close
        end
        flow do 
          para "Pick From:"
          @sw_thm = list_box items: items, choose: sel, margin_left: 5 
        end
        flow do
          button "Save", margin: 5 do
            f = File.new("#{path}/default", 'w')
            f.write @sw_thm.text
            f.close
          end
          button "Please - none of the above", margin: 5 do
            rm "#{path}/default"
          end
        end
        tagline "You have to quit and restart Shoes to see any changes"
        para "If the theme fails to load properly, it may faillback to" 
        para "ugly mode. Everything will be ok. Come back here and pick"
        para "a different theme, or none at all" 
      end
    end
  end
  
  def win_merge_screen
    @panel.clear
    @panel.append do
      stack do
        tagline "The will merge your app into a copy of Shoes for Windows"
        flow do 
          para "This avoids the dreaded Windows 10 Install problems and much "
          para "more. Shoes can be hidden from the user.  It won't conflict with "
          para "any other Shoes they might have. It strips out the manual and other "
          para "developer things.  You get your very own installer which you can customze "
          para "your way. This is how you build a real app!"
        end
        flow do
          para "That means you have to do somme more work. You'll need to download and "
          para "install the Installer maker, NSIS-Unicode and the ResourceHacker programs"
        end
        utilp = "#{LIB_DIR}/package/util.yaml"
        if ! File.exist? utilp
          rsexe = "reshacker_setup.exe"
          nsexe = "Shoes-Nsis-3.03.exe"
          para "Where is ResourceHacker.exe?  "
          flow do
            @rhel = edit_line width:300
            button "Download" do
              mkdir_p File.dirname(utilp)
              downloader "https://shoes.mvmanila.com/public/util/#{rsexe}",
                "#{LIB_DIR}/package/#{rsexe}"
            end
            button "Install" do
              system "#{LIB_DIR}/package/#{rsexe}"
            end
            button "Select" do
              @rhel.text = ask_open_file
            end
          end
          para "Where is Shoes_Nsis\\bin\\makensis.exe?"
          flow do
            @nsel = edit_line width:300
            button "Download" do
              mkdir_p File.dirname(utilp)
              downloader "https://shoes.mvmanila.com/public/util/#{nsexe}",
                "#{LIB_DIR}/package/#{nsexe}"
             end
            button "install" do
              system "#{LIB_DIR}/package/#{nsexe}"
            end
            button "Select" do
               @nsel.text = ask_open_file
            end
          end
          button "Save Locations" do
            exe = {}
            exe['rhp'] = @rhel.text
            exe['nsp'] = @nsel.text
            mkdir_p File.dirname(utilp)
            File.open(utilp,"w") {|f| f.write(exe.to_yaml) }
          end
        end
        st = Shoes.settings
        button "Merge", margin: 5 do
            require "package/build-exe"
        end
        @info_panel = stack do
        end
        para " "
      end
    end
  end
  
  def downloader dnlurl, workpath
        @info_panel.clear
        @info_panel.append do 
        background "#eee".."#ccd"
        @dnlpanel = stack :margin => 10 do
          dld = nil
          @dnlmenu= para dnlurl, " [", link("cancel") { @dlnthr.exit }, "]", :margin => 0
          @dnlstat = inscription "Beginning transfer.", :margin => 0
          @dnlbar = progress :width => 1.0, :height => 14 
	        @dlnthr = download dnlurl, :save =>  workpath,
		       :progress => proc { |dl| 
		          @dnlstat.text = "Transferred #{dl.transferred} of #{dl.length} bytes (#{sprintf('%2i',dl.percent * 100)}%)"
		          @dnlbar.fraction = dl.percent 
		         },
		      :finish => proc { |dl| 
		          @dnlstat.text = "Download completed"
		        }
	      end
      end
  end
  
  def deb_merge_screen
    @panel.clear
    @panel.append do
      stack do
        tagline "Linux - Merge your app into Shoes and create a .deb"
        flow do 
          para "You will need a png icon and license file. All field should \
be considered mandatory, Especially if you intend to submit your .deb to a site."
          para "You also need a Ruby installed with the fpm gem installed \
in that ruby. The merge will create the app directory and a script for you run \
to create the .deb"
        end
        # should check and only perform on Tight Shoes. (loose shoes seqfaults in copy)
        button "Merge" do
          #Shoes.setup do
          #  gem 'fpm'
          #end
          require "package/build-deb"
        end
      end
    end
  end
  
  def osx_merge_screen
    @panel.clear
    @panel.append do
      stack do
        tagline "OSX - Merge your app into Shoes and create a .dmg"
        flow do 
          para "You will need a png icon, a .icns image, a license file and \
background png image for the .dmg.  You will also need the `hdutil` program installed'.\
Also some patience: dmgs are not the quickest thing to build and its done silently"
        end
        button "Merge" do
          require "package/build-osx"
        end
      end
    end  
  end
  
  def bsd_merge_screen
    @panel.clear
    @panel.append do
      stack do
        tagline "Freebsd - Merge your app into Shoes and create package"
        flow do 
          para "Experiment"
        end
        # should check and only perform on Tight Shoes.
        button "Merge" do
          require "package/build-bsd"
        end
      end
    end
  end
end # App
