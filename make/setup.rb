# two functions in this module are used
#   env_create_and_save(build_os,target_os) - when `rake setup:xxxx`
#   Builder.load_env('build_target.yaml')   - when not above
#   
module Make
  include FileUtils
  APP = {}
  
  # get the app.yaml, git's idea of revision number,
  def self.basic_env(target_os)
    app = YAML.load_file(File.join(ENV['APP'] || ".", "app.yaml"))
    # APP['version'] = APP['major'] # for historical reasons
    # populate APP[] with uppercase names and string values
    APP['VERSION'] = "\"#{app['major']}.#{app['minor']}.#{app['tiny']}\""
    APP['MAJOR'] = app['major'].to_s
    APP['MINOR'] = app['minor'].to_s
    APP['TINY'] = app['tiny'].to_s
    APP['NAME'] = app['name']
    APP['release'] = app['release']
    APP['icons'] = app['icons']
    APP['DATE'] = Time.now.to_s
    APP['PLATFORM'] = RbConfig::CONFIG['arch'] # not correct for cross compile
    case app['revision']
      when 'git'
        gitp = ENV['GIT'] || "git"
        APP['REVISION'] = (`#{gitp} rev-list HEAD`.split.length).to_s
      when 'file'
        File.open('VERSION.txt', 'r') do |f|
          ln = f.read
          rev = ln[/r\(\d+\)/]
          APP['REVISION'] = "#{rev[/\d+/].to_i + 1}"
        end
      else
        if APP['revision'].kind_of? Fixnum
          APP['REVISION'] = APP['revision'].to_s
        else
          APP['REVISION'] = '9' # make it up
        end
    end
    
    
    APP['NAME'] = app['shortname'] || app['name'].downcase.gsub(/\W+/, '')
    APP['APPNAME'] = app['name'] # TODO: OSX needs this
    APP['SONAME'] = 'shoes'
    APP['APPARGS'] = APP['run']  # left over from HH build?
    
    # TODO: Shadow these until replaced with APP[] in env/setup/tasks files
    #RUBY_SO = RbConfig::CONFIG['RUBY_SO_NAME']
    APP['RUBY_SO'] = RbConfig::CONFIG['RUBY_SO_NAME']
    #RUBY_V = RbConfig::CONFIG['ruby_version']
    APP['RUBY_V'] = RbConfig::CONFIG['ruby_version']
    #SHOES_RUBY_ARCH = RbConfig::CONFIG['arch']
    APP['SHOES_RUBY_ARCH'] = RbConfig::CONFIG['arch']
    
    # default exts, gems & locations to build and include - replace with custom.yaml
    APP['GEMLOC'] = ""
    APP['EXTLOC'] = ""
    APP['EXTLIST'] = []
    APP['GEMLIST'] = []
    APP['Bld_Tmp'] = 'tmp'
    APP['Bld_Pre'] = ENV['NFS_ALTP'] if ENV['NFS_ALTP']
    APP['CROSS'] = true  # TODO constant escape from file. grrr
    if target_os =~ /yosemite|mavericks|minosx|darwin14/
      # osx is just different. It needs build performance optimizations 
      # is the build output directory outside the shoes3 dir?    
      if APP['Bld_Pre']
        APP['top_dir'] = APP['Bld_Pre']+target_os # likely to be abs path!
        APP['TGT_DIR'] = APP['top_dir']+"/#{APPNAME}.app/Contents/MacOS"
      else
        APP['TGT_DIR'] = target_os+"/#{APPNAME}.app/Contents/MacOS"
        APP['top_dir'] = APP['TGT_DIR']
      end
    else 
      # is the build output directory outside the shoes3 dir?    
      if APP['Bld_Pre']
        APP['TGT_DIR'] = APP['Bld_Pre']+target_os
      else
        APP['TGT_DIR'] = target_os
      end
    end
=begin
    mkdir_p "#{TGT_DIR}"
    BLD_ARGS = {}
    # This allows short circuiting the need to load setup and env (and pkg-config overhead)
    # Exprimental - no visible performance gain. see make/linux/minlin/env.rb
    if File.exists? "#{TGT_DIR}/build.yaml"
      $stderr.puts "loading building args"
      thsh = YAML.load_file("#{TGT_DIR}/build.yaml")
      thsh.each {|k,v| BLD_ARGS[k] = v} 
      HAVE_BLD = true
    else 
      HAVE_BLD = false
    end
=end
  end
  
  # Load <taget>-custom.yaml unless native 
  def self.load_custom(target_os, native=false)
    if native
      APP['CC'] = RbConfig::CONFIG['CC']
      APP['STRIP'] =RbConfig::CONFIG['STRIP']
      APP['RANBLIB'] = RbConfig::CONFIG['RANLIB']
      APP['ruby_pc'] = RbConfig::CONFIG['ruby_pc']
      APP['arch'] = RbConfig::CONFIG['arch']
      APP['RUBY_VERSION'] = RbConfig::CONFIG['ruby_version']
      APP['TGT_ARCH'] = target_os.to_s
      require File.expand_path("make/env/#{target_os}")
    else
      # nothing here YET - parse the custom.yaml and make/flags
    end
  end
  
  def self.env_create_and_save(build_os,target_os)
    # get the base stuff
    basic_env(target_os.to_s)
    # do specific setups
    case target_os
    when :minlin, :minbsd, :minosx
      #sh "echo 'TGT_ARCH=#{target_os}' >build_target"
      load_custom(target_os, true)
    else
      sh "echo 'TGT_ARCH=#{target_os}' >build_target"  
      #load_custom(target_os, false)    
    end
    # call Builder.static_setup -  
    require File.expand_path("make/#{build_os}/#{target_os}/setup")
    mkdir_p APP['TGT_DIR']
    Make.static_setup(APP)
    # Save APP hash to build_target.yaml
    File.open("build_target.yaml", 'w') { |f| YAML.dump(APP, f) }
  end
  
  def self.load_env(filename)
    # Load yaml into APP and create those CONSTANTS that
    # make us look like Makefiles. 
    app = YAML.load_file(filename)
    # Copy Make::APP to toplevel - better way? 
    hsh = Object.const_set('APP', app)  
    app.each do |k, v| 
      hsh[k] = v
    end
    Object.const_set('NAME', app['APPNAME'].downcase.gsub(/\W+/, ''))
    Object.const_set('TGT_DIR', app['TGT_DIR'])
    Object.const_set('TGT_ARCH' ,app['TGT_ARCH'])
    Object.const_set('DLEXT', app['DLEXT'])
    Object.const_set('SOLOCS', app['SOLOCS'])
    Object.const_set('SHOES_RUBY_ARCH', app['SHOES_RUBY_ARCH'])
    Object.const_set('CC', app['CC'])
    Object.const_set('LINUX_CFLAGS', app['LINUX_CFLAGS'])
    Object.const_set('LINUX_LIBS', app['LINUX_LIBS'])
    Object.const_set('LINUX_LDFLAGS', app['LINUX_LDFLAGS'])
    Object.const_set('EXT_RUBY', app['EXT_RUBY'])
    Object.const_set('CROSS', true)  # TODO - not needed in future. misnamed too!
    # Should set zzsetup.done here? 
  end
end # Module
