# This is a big gulp of copying.
require 'fileutils'
module Make
  include FileUtils
 
  def static_setup (so_list)
    $stderr.puts "setup: dir=#{`pwd`} for #{TGT_DIR}"
		#rm_rf "#{TGT_DIR} "
		mkdir_p "#{TGT_DIR}"
		# copy Ruby, dylib, includes - have them in place before
		# we build things 
		puts "Ruby at #{EXT_RUBY} : #{SHOES_GEM_ARCH} RUBY_V : #{RUBY_V}"
		rbvt = rbinc = APP['RUBY_V']
		rbvm = APP['RUBY_V'][/^\d+\.\d+/]
		rm_rf "#{TGT_DIR}/lib"
		mkdir_p "#{TGT_DIR}/lib"
		if rbvm == '2.3'
			cp_r "#{EXT_RUBY}/lib/ruby", "#{TGT_DIR}/lib"
			# copy and link libruby.dylib
			cp "#{EXT_RUBY}/lib/libruby.#{rbvm}.0.dylib", "#{TGT_DIR}"
			Dir.chdir(TGT_DIR) do
				 ln_s "libruby.#{rbvm}.0.dylib", "libruby.dylib"
			end
		else
		  # because ruby changes from version to version, os to os. Curses!
		  rbvt = RUBY_VERSION[/(\d+\.\d+\.\d+)/]
		  rbinc = rbvt[/\d+\.\d+/] + '.0'
			cp_r "#{EXT_RUBY}/lib/ruby", "#{TGT_DIR}/lib"
			# copy and link libruby.dylib
			cp "#{EXT_RUBY}/lib/libruby.#{rbvt}.dylib", "#{TGT_DIR}"
			Dir.chdir(TGT_DIR) do
				 ln_s "libruby.#{rbvt}.dylib", "libruby.#{rbvm}.dylib"
				 ln_s "libruby.#{rbvt}.dylib", "libruby.dylib"
			end
		end
		# copy include files - it might help build gems
		mkdir_p "#{TGT_DIR}/lib/ruby/include/ruby-#{rbinc}"
		cp_r "#{EXT_RUBY}/include/ruby-#{rbinc}/", "#{TGT_DIR}/lib/ruby/include"
		
		if APP['LIBPATHS']
		  dep_find_and_copy( APP['LIBPATHS'], SOLOCS)
		else
		  SOLOCS.each do |k, v| 
		    cp v, TGT_DIR
		  end
		end
		# copy ssl engines
		if File.exist?("#{ShoesDeps}/lib/engines")
			cp_r "#{ShoesDeps}/lib/engines", "#{TGT_DIR}"
		end
    
    # copy some static stuff
    cp_r  "fonts", "#{TGT_DIR}/fonts"
    cp_r  "lib", "#{TGT_DIR}"
    cp_r  "samples", "#{TGT_DIR}/samples"
    cp_r  "static", "#{TGT_DIR}/static"
    cp    "README.md", "#{TGT_DIR}/README.txt"
    cp    "CHANGELOG", "#{TGT_DIR}/CHANGELOG.txt"
    cp    "COPYING", "#{TGT_DIR}/COPYING.txt"
	end
end

