# -*- encoding: utf-8 -*-
#
# lib/shoes.rb
# The Shoes base app, both a demonstration and the learning tool for
# using Shoes.
#

require_relative 'shoes/cache' # do First thing
ARGV.delete_if { |x| x =~ /-psn_/ }

# Probably don't need this
class Encoding
  %w(UTF_7 UTF_16BE UTF_16LE UTF_32BE UTF_32LE).each do |enc|
    eval "class #{enc};end" unless const_defined? enc.to_sym
  end
end

require 'open-uri'
require 'optparse'
require 'resolv-replace' if RUBY_PLATFORM =~ /win/
require_relative 'shoes/inspect'
require_relative 'shoes/image' if Object.const_defined? :Shoes

def Shoes.hook; end

# class Encoding
# %w[ASCII_8BIT UTF_16BE UTF_16LE UTF_32BE UTF_32LE US_ASCII].each do |ec|
#   eval "#{ec} = '#{ec.sub '_', '-'}'"
# end unless RUBY_PLATFORM =~ /linux/ or RUBY_PLATFORM =~ /darwin/ or RUBY_PLATFORM =~ /mingw/
# end

class Range
  def rand
    conv = (Integer === self.end && Integer === self.begin ? :to_i : :to_f)
    ((Kernel.rand * (self.end - self.begin)) + self.begin).send(conv)
  end
end

unless Time.respond_to? :today
  def Time.today
    t = Time.now
    t - (t.to_i % 86_400)
  end
end

class Shoes
  RELEASES = %w(Curious Raisins Policeman Federales Walkabout)

  NotFound = proc do
    para '404 NOT FOUND, GUYS!'
  end

  class << self; attr_accessor :locale, :language end
  @locale = ENV['SHOES_LANG'] || ENV['LC_MESSAGES'] || ENV['LC_ALL'] || ENV['LANG'] || 'C'
  @language = @locale[/^(\w{2})_/, 1] || 'en'

  @mounts = []

  OPTS = OptionParser.new do |opts|
    opts.banner = "Usage: shoes [options] (app.rb or app.shy)"

    opts.on('-d', '--debug', 'Debug Shoes script') do
      ENV['CMDLINE_DEBUG'] = true.to_s
    end

    opts.on("-m", "--manual",
            "Open the built-in manual.") do
      show_manual
    end

    opts.on("-w", "--console", "display console") do
      if ENV['console_loop']
        show_log # need something on the screen for Shoes- FIXME
        show_console
        require 'readline'
        require 'io/console'
        Thread.new do
          loop do
            #$stdout.write "prompt: "
            #ln = $stdin.gets
            #ln = $stdin.readline
            #Readline::vi_editing_mode
            ln = Readline::readline('> ', false)
            #ln = STDIN.cooked(&:gets)

            if ln.strip == 'quit'
              $stderr.write "really quit (y/n)"
              ans = $stdin.gets.strip
              exit if ans == 'y'
            end
            $stdout.puts "Shoes: #{ln}"
          end
        end
      else
        #ENV['SHOES_CONSOLE'] = true.to_s
        show_console
      end
    end

    opts.on("--old-package",
            "(Obsolete) Package a Shoes app for Windows, OS X and Linux.") do |s|
      make_pack
    end

    opts.on("-c", "--cobbler",
            "Maintain Shoes installation") do |c|
      cobbler
    end

    opts.on("-p", "--package",
            "Package Shoes App (new)") do |c|
      app_package
    end
    
    opts.on('-g', '--gem',
            'Passes commands to RubyGems.') do
      require 'shoes/setup'
      require 'rubygems/gem_runner'
      Gem::GemRunner.new.run(ARGV)
      fail SystemExit, ''
    end

    opts.on('--manual-html DIRECTORY', 'Saves the manual to a directory as HTML.') do |dir|
      #manual_as :html, dir
      fail SystemExit, "HTML manual in: #{dir}"
    end

    opts.on('--install MODE SRC DEST', 'Installs a file.') do |mode|
      src, dest = ARGV
      FileUtils.install src, dest, mode: mode.to_i(8), preserve: true
      fail SystemExit, ''
    end

    opts.on('--nolayered', 'No WS_EX_LAYERED style option.') do
      $NOLAYERED = 1
      Shoes.args!
    end
    
    opts.on('-f', '--file', 'path to script [OSX packaging uses this]') do
      #puts "-f ARGV: #{ARGV}"
    end
    
    opts.on_tail('-v', '--version', 'Display the version info.') do
      # raise SystemExit, File.read("#{DIR}/VERSION.txt").strip
      # str = "Shoes #{Shoes::VERSION_NAME} r#{Shoes::VERSION_REVISION} #{Shoes::VERSION_DATE} #{RUBY_PLATFORM} #{RUBY_VERSION}"
      # $stderr.puts str
      fail SystemExit, "Shoes #{Shoes::VERSION_NAME} #{Shoes::VERSION_NUMBER} r#{Shoes::VERSION_REVISION} #{RUBY_PLATFORM} #{RUBY_VERSION}"
    end

    opts.on_tail('-h', '--help', 'Show this message') do
      fail SystemExit, opts.to_s
    end
  end

  class SettingUp < StandardError; end

  @setups = {}

  def self.setup(&blk)
    require 'shoes/setup'
    line = caller[0]
    return if @setups[line]
    script = line[/^(.+?):/, 1]
    set = Shoes::Setup.new(script, &blk)
    @setups[line] = true
    fail SettingUp unless set.no_steps?
  end

  def self.show_selector (debug = false)
    fname = ask_open_file
    Shoes.visit(fname, debug) if fname
  end

  def self.package_app
  end

  def self.splash
  end

  def self.cobbler
    #require 'shoes/cobbler'
  end

  def self.app_package
    #require 'shoes/app_package'
  end

  def self.make_pack
    #require 'shoes/pack'
    #Shoes.app(width: 500, height: 480, resizable: true, &PackMake)
  end


  def self.show_manual
    #manual_as :shoes
  end

  def self.show_irb
    #require 'shoes/irb'
    #Shoes.irb
  end

  def self.remote_debug
    #require "shoes/remote_debugger.rb"
    #Shoes.rdb
  end

  def self.show_log
    require 'shoes/log'
    return if @log_app && Shoes.APPS.include?(@log_app)
    @log_app =
      Shoes.app do
        extend Shoes::LogWindow
        setup
      end
  end
  
  def self.mount(path, meth, &blk)
    unless @mounts.empty?
      # checking if app changed
      @mounts.clear if meth[0] != @mounts[0][1][0]
    end
    @mounts << [path, meth || blk]
  end

  # SHOES_URL_RE = %r!^@([^/]+)(.*)$!
  SHOES_URL_RE = %r{^@([^/]+)(.*)$}

  def self.run(path)
    uri = URI(path)
    @mounts.each do |mpath, rout|
      m, *args = *path.match(/^#{mpath}$/)
      if m
        rout = rout[0].instance_method(rout[1]) unless rout.is_a? Proc
        # return [rout, args]
        return [rout, args, rout.owner] # requires change in app.c
      end
    end
    case uri.path when '/'
                    [nil]
    when SHOES_URL_RE
      [proc { eval(URI("http://#{Regexp.last_match(1)}:53045#{Regexp.last_match(2)}").read) }]
    else
      [NotFound]
    end
  end

  def self.args!
    #Shoes.splash if RUBY_PLATFORM !~ /darwin/ && ARGV.empty?
    #OPTS.parse! ARGV
    #ARGV[0] || true
    font "#{DIR}/fonts/Lacuna.ttf"
    real_path = "#{DIR}"
    app_path = "#{real_path}/#{APP_START}"
  end

  def self.uri(str)
    if str =~ SHOES_URL_RE
      URI("http://#{Regexp.last_match(1)}:53045#{Regexp.last_match(2)}")
    else
      URI(str) rescue nil
    end
  end

  def self.visit(path, debug=false)
    uri = Shoes.uri(path)
    case uri
    when URI::HTTP
      str = uri.read
      if str !~ /Shoes\.app/
        Shoes.app do
          eval(uri.read)
        end
      else
        eval(uri.read)
      end
    else
      path = File.expand_path(path.gsub(/\\/, '/'))
      if path =~ /\.shy$/
        @shy = true
        require 'shoes/shy'
        base = File.basename(path, '.shy')
        #@tmpdir = tmpdir = '%s/shoes-%s.%d' % [Dir.tmpdir, base, $PROCESS_ID]
        @tmpdir = tmpdir = "%s/shoes-%s.%d" % [Dir.tmpdir, base, $$]
        shy = Shy.x(path, tmpdir)
        Dir.chdir(tmpdir)
        # Shoes.debug "Loaded SHY: #{shy.name} #{shy.version} by #{shy.creator}"
        path = shy.launch
      else
        @shy = false
        Dir.chdir(File.dirname(path))
        path = File.basename(path)
      end
      if ENV['CMDLINE_DEBUG']
        require 'byebug'
        require 'byebug/runner'
        $PROGRAM_NAME = path
        Byebug.debug_load($PROGRAM_NAME, true) # this starts byebug loop
      elsif debug
        # spin up the console window and call the debugger with the path
        require 'shoes/debugger'
        @console_app =
          Shoes.app do
            extend Shoes::Debugger
            setup path
          end
      else
        $0.replace path
        code = read_file(path)
        eval(code, TOPLEVEL_BINDING, path)
      end
    end
  rescue SettingUp
  rescue Object => e
    error(e)
    show_log
  end

  def self.clean
    if @shy
      Dir.chdir() # do it from HOME 
      FileUtils.rm_rf(@tmpdir, secure: true)
    end
  end

  def self.read_file(path)
    if RUBY_VERSION =~ /^1\.9/ && !@shy
      # File.open(path, 'r:utf-8') { |f| f.read }
      IO.read(path).force_encoding('UTF-8')
    elsif RUBY_VERSION =~ /^2\.0/ && !@shy
      IO.read(path).force_encoding('UTF-8')
    else
      File.read(path)
    end
  end

  def self.url(path, meth)
    Shoes.mount(path, [self, meth])
  end

  module Basic
    def tween(opts, &blk)
      opts = opts.dup

      if opts[:upward]
        opts[:top] = top - opts.delete(:upward)
      elsif opts[:downward]
        opts[:top] = top + opts.delete(:downward)
      end

      if opts[:sideways]
        opts[:left] = self.left + opts.delete(:sideways)
      end

      @TWEEN.remove if @TWEEN
      @TWEEN = parent.animate(opts[:speed] || 20) do
        # figure out a coordinate halfway between here and there
        cont = opts.select do |k, v|
          if self.respond_to? k
            n = v
            o = send(k)
            if n != o
              n = o + ((n - o) / 2)
              n = v if o == n
              send("#{k}=", n)
            end
            style[k] != v
          end
        end

        # if we're there, get rid of the animation
        if cont.empty?
          @TWEEN.remove
          @TWEEN = nil
          blk.call if blk
        end
      end
    end
  end

  # complete list of styles
  BASIC_S = [:left, :top, :right, :bottom, :width, :height, :attach, :hidden,
             :displace_left, :displace_top, :margin, :margin_left, :margin_top,
             :margin_right, :margin_bottom]
  TEXT_S  = [:strikecolor, :undercolor, :font, :size, :family, :weight,
             :rise, :kerning, :emphasis, :strikethrough, :stretch, :underline,
             :variant]
  MOUSE_S = [:click, :motion, :release, :hover, :leave]
  KEY_S   = [:keydown, :keypress, :keyup]
  COLOR_S = [:stroke, :fill]

  { Background => [:angle, :radius, :curve, *BASIC_S],
    Border     => [:angle, :radius, :curve, :strokewidth, *BASIC_S],
    Canvas     => [:scroll, :start, :finish, *(KEY_S | MOUSE_S | BASIC_S)],
    Check      => [:click, :checked, *BASIC_S],
    Radio      => [:click, :checked, :group, *BASIC_S],
    EditLine   => [:change, :secret, :text, *BASIC_S],
    EditBox    => [:change, :text, *BASIC_S],
    Effect     => [:radius, :distance, :inner, *(COLOR_S | BASIC_S)],
    Image      => MOUSE_S | BASIC_S,
    ListBox    => [:change, :items, :choose, *BASIC_S],
    # Pattern    => [:angle, :radius, *BASIC_S],
    Progress   => BASIC_S,
    Shape      => COLOR_S | MOUSE_S | BASIC_S,
    TextBlock  => [:justify, :align, :leading, *(COLOR_S | MOUSE_S | TEXT_S | BASIC_S)],
    Text       => COLOR_S | MOUSE_S | TEXT_S | BASIC_S }
    .each do |klass, styles|
    klass.class_eval do
      include Basic
      styles.each do |m|
        case m when *MOUSE_S
               else
                 define_method(m) { style[m] } unless klass.method_defined? m
                 define_method("#{m}=") { |v| style(m => v) } unless klass.method_defined? "#{m}="
        end
      end
    end
  end

  class Types::Widget
    @types = {}
    def self.inherited(subc)
      methc = subc.to_s[/(^|::)(\w+)$/, 2]
              .gsub(/([A-Z]+)([A-Z][a-z])/, '\1_\2')
              .gsub(/([a-z\d])([A-Z])/, '\1_\2').downcase
      @types[methc] = subc
      Shoes.class_eval %{
        def #{methc}(*a, &b)
          a.unshift Widget.instance_variable_get("@types")[#{methc.dump}]
          widget(*a, &b)
        end
      }
    end
  end
end

def window(*a, &b)
  Shoes.app(*a, &b)
end
