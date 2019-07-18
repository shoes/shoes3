module Make
  include FileUtils

  def cc(t)
    sh "#{CC} -I. -c -o#{t.name} #{LINUX_CFLAGS} #{t.source}"
  end

  # Subs in special variables
  def rewrite before, after, reg = /\#\{(\w+)\}/, reg2 = '\1'
    File.open(after, 'w') do |a|
      File.open(before) do |b|
        b.each do |line|
          a << line.gsub(reg) do
            if reg2.include? '\1'
              reg2.gsub(%r!\\1!, Object.const_get($1))
            else
              reg2
            end
          end
        end
      end
    end
  end
end

require_relative '../package'

include FileUtils

class MakeLinux
  extend Make

  class << self
    def setup_system_resources
      cp APP['icons']['gtk'], "#{TGT_DIR}/static/app-icon.png"
    end

    def new_so (name) 
      $stderr.puts "new_so: #{name}"
      tgtd = File.dirname(name)
      objs = []
      SubDirs.each do |f|
        d = File.dirname(f)
        objs = objs + FileList["#{d}/*.o"]      
      end
      objs = objs + FileList["shoes/native/gtk/*.o"]
      main_o = 'shoes/main.o'
      objs = objs - [main_o]
      sh "#{CC} -o  #{tgtd}/libshoes.#{DLEXT} #{objs.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}" 
    end
    
    def new_link(name)
      #name is actually a file path
      $stderr.puts "new_link: arg=#{name}"
      dpath = File.dirname(name)
      fname = File.basename(name)
      bin = "#{fname}-bin"
      sh "#{CC} -o #{dpath}/#{bin} #{dpath}/tmp/main.o -lpthread -lresolv -L#{dpath} -lshoes -L#{TGT_DIR}  #{LINUX_LIBS} --sysroot=#{ShoesDeps}"
      rewrite "platform/nix/shoes.launch", name, %r!/shoes-bin!, "/#{NAME}-bin"
      sh %{echo 'cd "$OLDPWD"\nLD_LIBRARY_PATH=$APPPATH $APPPATH/#{File.basename(bin)} "$@"' >> #{name}}
      chmod 0755, "#{name}" 
      # write a gdb launched shoes
      rewrite "platform/nix/shoes.launch", "#{TGT_DIR}/debug", %r!/shoes-bin!, "/#{NAME}-bin"
      sh %{echo 'cd "$OLDPWD"\nLD_LIBRARY_PATH=$APPPATH gdb $APPPATH/#{File.basename(bin)} "$@"' >> #{TGT_DIR}/debug}
      chmod 0755, "#{TGT_DIR}/debug" 
    end
    
    def make_installer
      if APP['INSTALLER'] == 'appimage'
        make_installer_appimage 'shoes', 'armhf', 'static/app-icon.png'
      else
        make_installer_makeself
      end
    end
    
   
    # run strip on the libraries, remove unneeded ruby code (tk,
    #  readline and more)
    def make_smaller
      puts "Shrinking #{`pwd`}"
      sh "#{STRIP} *.so"
      sh "#{STRIP} *.so.*"
      Dir.glob("lib/ruby/**/*.so").each {|lib| sh "#{STRIP} #{lib}"}
    end
  end
end
