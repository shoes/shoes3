module Make
  include FileUtils


  # Set up symlinks to lib/shoes and lib/shoes.rb so that they
  # can be edited and tested without a rake clean/build every time we 
  # change a lib/shoes/*.rb  
  # They'll be copied (not linked) when rake install occurs. Be very
  # careful. Only Link to FILES, not to directories. Fileutils.ln_s may
  # not be the same as linux ln -s. 
  def static_setup (solocs)
    srcloc= `pwd`.strip
    puts "setup: #{srcloc}"
    
    ln_s "#{srcloc}/lib", TGT_DIR
    ln_s "#{srcloc}/samples", TGT_DIR
    ln_s "#{srcloc}/static",  TGT_DIR
    ln_s "#{srcloc}/fonts", TGT_DIR
    ln_s "#{srcloc}/themes", TGT_DIR
    cp    "README.md", TGT_DIR
    cp    "CHANGELOG", TGT_DIR
    cp    "COPYING", TGT_DIR
  end
end
