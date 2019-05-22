# build Shoes gems to be copied later. Fluff is not copied - Just the
# gemspec, LICENSE lib/ and the right arch so/dll/dylib
# NOTE: in Shoes 3.3.8 gem handling
require 'rubygems'
require 'rubygems/dependency_installer'

# because you can't have too much meta-programming hacking
class  << Gem::Platform
  def change_platform(arch_str)
    @local = new(arch_str)
  end
end

module Make
  include FileUtils
  Lext = DLEXT == 'dylib'? 'bundle' : 'so'

  # note: Some methods are unique to invoking context and
  # may not have the Constants set for other contexts
  
  def self.build_shoes_gem (name, version)
    if !version 
      version = Gem::Requirement.default
    end
    # Tell Gem:: what platform we want instead of what we are
    Gem::Platform.change_platform(RbConfig::CONFIG['arch'])
    installer = Gem::DependencyInstaller.new domain: :remote
    begin
      poss_gems = installer.find_spec_by_name_and_version(name, version)
    rescue Gem::SpecificGemNotFoundException => e
      $stderr.puts "Failed to find a spec: #{e}"
      abort
    end
    # a temp place to put cross compiled gems (like GEM_HOME but not!)
    tmpdir = 'gemtemp'
    mkdir_p tmpdir
    best_spec = nil;
    poss_gems.match_platform! # doesn't help
    poss_gems.each_spec do |spec|
      $stderr.puts "and  arch: #{Gem::Platform.local}"
      require_relative 'ext_conf_builder.rb'  # our monkey patch
      $make_dir = Dir.getwd + '/make'
      installer = Gem::DependencyInstaller.new({:document => [], :install_dir => tmpdir})
      installer.install(spec.name, spec.version)
    end
    # now extract the (new) gem to abbreviated Shoes version in Gemloc
  end

    def build_shoes_ext xdir
      gemn = File.basename(xdir)
      puts "Building ext #{gemn} in #{xdir} from #{`pwd`}"
      dest = "#{APP['EXTLOC']}/built/#{TGT_ARCH}/#{gemn}"
      rm_rf dest
      mkdir_p dest
      Dir.glob("#{xdir}/ext/*").each do |ext|
        puts "compile #{ext}"
        Dir.chdir(ext) do
          extcnf = (File.exists? "#{TGT_ARCH}-extconf.rb") ? "#{TGT_ARCH}-extconf.rb" : 'extconf.rb'
          unless system "ruby", "#{extcnf}" and system "make"
            raise "Gem build failed"
        end
        Dir.glob("*.#{Lext}").each do |so|
          mkdir_p "#{dest}/ext"
          cp so, "#{dest}/ext"
        end
       end
      end
      cp_r "#{xdir}/lib/", dest if File.exists? "#{xdir}/lib"
    end

    def build_gem xdir
      gemn = File.basename(xdir)
      puts "Building gem #{gemn} in #{xdir} from #{`pwd`}"
      Dir.glob("#{xdir}/ext/*").each do |ext|
        Dir.chdir(ext) do
          extcnf = (File.exists? "#{TGT_ARCH}-extconf.rb") ? "#{TGT_ARCH}-extconf.rb" : 'extconf.rb'
          unless system "ruby", "#{extcnf}" and system "make"
            raise "Gem build failed"
          end
        end
      end
      # copy to GEMLOC/built/TGT_ARCH/gemname for later inclusion
      # into shoes build, package, gempack
      dest = "#{APP['GEMLOC']}/built/#{TGT_ARCH}/#{gemn}"
      rm_rf dest
      mkdir_p dest
      cp "#{xdir}/gemspec", dest
      # hack to keep spec and gem separate - May 1, 2015
      spec = eval(File.read("#{xdir}/gemspec"))
      dest << "/#{spec.full_name}"
      mkdir_p dest
      cp "#{xdir}/LICENSE", dest
      cp_r "#{xdir}/lib/", dest
      Dir.glob("#{xdir}/ext/*/*.#{Lext}").each {|so| cp so, "#{dest}/lib" }
    end

    def clean_all_gems
      Dir.glob("#{APP['GEMLOC']}/*").each do |gemp|
        next if gemp =~ /built$/
        puts "clean #{gemp}"
        clean_gem gemp
      end
      rm_rf "#{APP['GEMLOC']}/built/#{TGT_ARCH}"
    end

    def clean_gem gemp
      gemn = File.basename(gemp)
      Dir.glob("#{gemp}/ext/*").each do |ext|
        Dir.chdir(ext) do |f|
          Dir.glob('*.o').each {|f| rm_r f}
          Dir.glob('*.so').each {|f| rm_r f}
          Dir.glob('*.bundle').each {|f| rm_r f}
          Dir.glob('*.def').each {|f| rm_r f}
        end
      end
    end

      def copy_files glob, dir
    FileList[glob].each { |f| cp_r f, dir }
  end

  #  ---- this method is called from the shoes build
  def copy_gems
    puts "copy_gems dir=#{pwd} #{TGT_ARCH}"
    if APP['RVMGEM'] 
      # only minosx target does this and that's iffy
      Dir.glob("#{APP['RVMGEM']}/*") do |f|
        p = "#{APP['RVMGEM']}/#{File.basename(f)}"
        cp_r p, "#{TGT_DIR}/lib/ruby/gems/#{APP['RUBY_V']}"
      end
      return 
    end
=begin
    APP['EXTLIST'].each do |ext|
      $stderr.puts "copy prebuild ext #{ext}"
      copy_files "#{APP['EXTLOC']}/built/#{TGT_ARCH}/#{ext}/ext/*.#{Lext}", "#{TGT_DIR}/lib/ruby/#{RUBY_V}/#{SHOES_TGT_ARCH}"
      if  File.exists? "#{APP['EXTLOC']}/built/#{TGT_ARCH}/#{ext}/lib"
        Dir.glob("#{APP['EXTLOC']}/built/#{TGT_ARCH}/#{ext}/lib/*").each do |lib|
          cp_r lib, "#{TGT_DIR}/lib/ruby/#{RUBY_V}"
        end
      end
    end
=end
    gdir = ""
    if RUBY_PLATFORM =~ /bsd/
      #gdir = "#{TGT_DIR}/lib/ruby/gems/#{RUBY_V}.0"
      gdir = "#{TGT_DIR}/lib/ruby/gems/#{APP['RUBY_V']}"
    else
      gdir = "#{TGT_DIR}/lib/ruby/gems/#{APP['RUBY_V']}"
    end
    # precompiled gems here - just copy
    APP['INCLGEMS'].each do |gemn|
      gemp = "#{APP['GEMLOC']}/built/#{TGT_ARCH}/#{gemn}"
      if ! File.exist?(gemp)
        pos = Dir.glob("#{APP['GEMLOC']}/built/#{TGT_ARCH}/#{gemn}")
        if pos && pos.length == 1
          gemp = pos[0]
          gemn = File.basename(gemp)
        else
          $stderr.puts "Failed to find #{gemn}. Is wildcard correct? "
          abort
        end
      end
      $stderr.puts "Copying prebuilt gem #{gemp} for #{SHOES_GEM_ARCH}"
      spec = eval(File.read("#{gemp}/gemspec"))
      mkdir_p "#{gdir}/specifications"
      mkdir_p "#{gdir}/specifications/default"
      # newer gempack compatible directory layout ?
      if spec.full_name == gemn #newer if true
        mkdir_p "#{gdir}/gems/#{spec.full_name}"
        if File.exists? File.join(gemp, 'extensions')
          # new style with shoes 3.3.8
          rubyv = APP['RUBY_V'][/\d.\d/]+'.0'
          gemcompl = File.join(gdir, 'extensions', SHOES_GEM_ARCH,
              rubyv)
          mkdir_p gemcompl
          cp_r File.join(gemp, 'extensions', spec.full_name), gemcompl
        elsif File.exists? File.join(gemp, 'gem.build_complete')
          #rubyv = RUBY_VERSION[/\d.\d/]+'.0'
          rubyv = APP['RUBY_V'][/\d.\d/]+'.0'
          gemcompl = File.join(gdir, 'extensions', SHOES_GEM_ARCH,
              rubyv, spec.full_name)
          mkdir_p gemcompl
          cp File.join(gemp, 'gem.build_complete'), gemcompl
        end
        cp "#{gemp}/gemspec", "#{gdir}/specifications/#{spec.full_name}.gemspec"
        if spec.require_paths.include? 'ext'
          $stderr.puts "Ext weird copy = #{spec.require_paths}"
          mkdir_p "#{gdir}/gems/#{spec.full_name}/ext"
          cp_r "#{gemp}/ext",  "#{gdir}/gems/#{spec.full_name}"
        end
        if spec.require_paths.include? 'lib'
          $stderr.puts "Lib copy = #{spec.require_paths[-1]}"
          mkdir_p "#{gdir}/gems/#{spec.full_name}/lib"
          cp_r "#{gemp}/lib",  "#{gdir}/gems/#{spec.full_name}"
        end
      else
        # Just copy lib - bin and doc and such don't matter to Shoes.
        mkdir_p "#{gdir}/gems/#{spec.full_name}/lib"
        cp_r "#{gemp}/#{spec.full_name}/lib",  "#{gdir}/gems/#{spec.full_name}"
        # we put the gem in default - kind of makes sense.
        cp "#{gemp}/gemspec", "#{gdir}/specifications/default/#{spec.full_name}.gemspec"
      end
      # do we need a rpath fixup? linux? probably not. OSX possibly
      if false
        FileList["#{gdir}/gems/#{spec.full_name}/**/*.so"].each do |so|
          $stderr.puts "FIX rpath #{so}"
          sh "chrpath #{so} -r '${ORIGIN}/../lib'"
        end
      end
      # Deal with precompiled native gems (usually from rakecompiler)'
      # Delete the versioned native libs we can't use
      if spec.full_name  =~ /nokogiri-(\d+.\d+.\d+.\d+)-(x86|x64)-mingw32/ 
        grubyv = APP['RUBY_V'][/\d.\d/]
        Dir.chdir("#{gdir}/gems/#{spec.full_name}/lib/#{spec.name}/") do
          Dir.glob('*').each do |dirn|
            if dirn =~ /\d.\d/ && dirn != grubyv
              $stderr.puts "Noko delete: #{dirn}"
              rm_r dirn
            end
          end
        end
      end
    end
  end
  
  # str may have digits and such in it.
  def self.copy_to_gemloc(str)
    name = str[/\w+/]
    gems = Dir.glob("gemtemp/specifications/#{name}*")
    if gems.size > 1
      #narrow it down
      $stderr.puts "Too many gems starting with #{name}"
      abort
    end
    if gems.size != 1
      $stderr.puts "Couldn't find a match for #{str}"
      abort
    end
    # spec is eval'ed as rake ruby so the file paths are likely wrong
    # for this purpose
    spec = eval(File.read(gems[0]))
    gemname = File.basename(gems[0])
    gemtemp = 'gemtemp'  # TODO be less fixed
    destpath = File.join(APP['GEMLOC'], 'built', TGT_ARCH, File.basename(spec.full_name))
    $stderr.puts "Copy #{spec.full_name} to #{destpath}"
    mkdir_p destpath
    # copy spec
    cp    File.join(gemtemp,'specifications', gemname), File.join(destpath,'gemspec')
    
    # deal with gem.build_complete. 
    # In shoes 3.3.8 we copy all of the extention dir
    rubyv = RUBY_VERSION[/\d.\d/]+'.0'
    gemcompl = File.join(gemtemp, 'extensions', "#{Gem::Platform.local}",
         rubyv, spec.full_name)
    #puts "Check for #{gemcompl}" 
    if File.exist? File.join(gemcompl,'gem.build_complete')
        #cp File.join(gemcompl,'gem.build_complete'), File.join(destpath,'gem.build_complete')
        mkdir_p File.join(destpath, 'extensions')
        cp_r gemcompl, File.join(destpath, 'extensions')
    end 

    # now copy the lib/ and any native library 
    # require paths is an array
    reqpath = spec.require_paths
    #puts "spec.require_paths: #{reqpath}"
    if (reqpath.length > 1) &&  (reqpath.include? 'ext')
      # native library is in ext
      src = File.join(reqpath)
      dest = File.join(destpath, reqpath[1])
      mkdir_p dest
      $stderr.puts "ext copy #{src} -> #{dest}"
      #cp_r src, dest
      cp_r File.join(gemtemp,'gems', spec.full_name, 'ext'), destpath
    elsif (reqpath.length > 1) &&  (reqpath.include? 'lib')
      $stderr.puts "lib copy  #{}"
      cp_r File.join(gemtemp,'gems', spec.full_name, 'lib'), destpath
    else
      reqpath.each do |rqp| 
        $stderr.puts "misc copy #{rqp}"
        cp_r File.join(gemtemp,'gems', spec.full_name, rqp), destpath
      end
    end
  end
    
  # prebuilt gem
  def self.fetch_native(str)
    $stderr.puts "looking for prebuilt gem"
    flds = str.split('-')
    if flds.length != 4
      $stderr.puts "Must be name-version-platform"
      $stderr.puts flds.inspect
      abort
    end
    cmd = "gem install #{flds[0]} --no-doc --version #{flds[1]} \
--platform #{flds[2]}-#{flds[3]} --install-dir gemtemp"
    $stderr.puts "using #{cmd}"
    system cmd
  end
end

namespace :gems do

  desc "build all gems for platform"
  task :buildall  do
    APP['INCLGEMS']
    Make::build_all_gems()
  end
  
  desc "Build internal Shoes gems <arg>"
  task :build do
    if ARGV.length < 2 || ARGV.length > 3
      $stderr.puts "Use one argument with optiobal version"
      abort
    end
    if ARGV[1][/mingw/]
      # we can download a prebuilt native gem for a different platform.
      Make::fetch_native(ARGV[1])
    else
      Make::build_shoes_gem(ARGV[1], ARGV[2])
    end
    Make::copy_to_gemloc(ARGV[1])
    # TODO: I use abort here because ARGV[1]+ is already in Rake's task list. 
    abort
  end
  
  desc "clean all gems for platform"
  task :clean do
   Make::clean_all_gems
  end
  
  
end

