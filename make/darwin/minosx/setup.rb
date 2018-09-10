require 'fileutils'
module Make
  include FileUtils
 
  # Set up symlinks to lib/shoes and lib/shoes.rb. DO NOT create
  # symkinks to dylibs

  def static_setup (solocs)
    srcloc= `pwd`.strip
    tgtloc = TGT_DIR
    mkdir_p tgtloc
    $stderr.puts "setup: #{srcloc}"
    ln_s "#{srcloc}/lib", tgtloc
    ln_s "#{srcloc}/samples", tgtloc
    ln_s "#{srcloc}/static",  tgtloc
    ln_s "#{srcloc}/fonts", tgtloc
    ln_s "#{srcloc}/themes", tgtloc
    cp    "README.md", tgtloc
    cp    "CHANGELOG", tgtloc
    cp    "COPYING", tgtloc
    # be very carefull with symlinks into ruby
    #ln_s "#{EXT_RUBY}/lib/ruby", "#{tgtloc}/lib/ruby"
  end

end

