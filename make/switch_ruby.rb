# Call this to switch RUBY_VERSION constant to what's inside the
# specified rbconfig.rb.
def switch_ruby(rbpath)
  cnt = 0
  rbv = ""
  IO.foreach(rbpath) do |ln|
    cnt += 1
    if ln[/(\(\d.\d.\d\))/]
      rbv = "#{$1[1..-2]}"
      break
    end
    if cnt > 15 
      puts "Could not find version (d.d.d) in first 15 lines"
      abort
    end
  end
  RbConfig.module_eval { remove_const :CONFIG }
  RbConfig.module_eval { remove_const :TOPDIR }
  RbConfig.module_eval { remove_const :MAKEFILE_CONFIG }

  Object.instance_eval{ remove_const :RUBY_VERSION }
  Object.const_set("RUBY_VERSION", rbv)
  puts "Using Ruby version #{RUBY_VERSION}"
  # remove rbconfig.rb from $" list.
  #$".delete("#{td}/rbconfig.rb")
  #puts $".inspect
end
rbcfg = Dir.glob("#{EXT_RUBY}/lib/ruby/**/*/rbconfig.rb")[0]
# RVM id kind of weird for me. It can load the wrong rbconfig? Just Odd.
# Make sure we aren't replacing The rake running Ruby with itself.
td = "#{RbConfig::TOPDIR}/lib/ruby/#{RbConfig::CONFIG['ruby_version']}/#{RbConfig::CONFIG['arch']}/rbconfig.rb"
if rbcfg != td
  switch_ruby(rbcfg)
  if !require rbcfg
    puts "Failed to load #{rbcfg}, current: #{td}"
  end
end
