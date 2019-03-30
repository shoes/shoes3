#! /usr/bin/env ruby
# ./fixstruct.rb NEW_MACRO_CANVAS shoes_canvas ../types/*.c

require 'fileutils'
include FileUtils

argc = ARGV.length - 2
if argc <= 0 
  puts "incomplete arguments, <macro name> <strut_name> files*"
  exit
end
shoes_macro = ARGV[0]
shoes_pat = ARGV[1]
argc.times do |fi| 
  fn = ARGV[fi+2]
  puts  "processing #{shoes_pat} in #{fn}"
  p = File.dirname(fn)
  fon = "#{p}/#{File.basename(fn)}.new"
  fpn = "#{p}/#{File.basename(fn)}.orig"
  fi = File.new(fn)
  fo = File.new(fon, 'w')
  fi.each do |ln| 
    if ln[/(.*)Data_Get_Struct\((.+,\ )#{shoes_pat},\ (\w+)/]
      lead = $1
      obj = $2
      fo.puts "\#ifdef #{shoes_macro}"
      fo.puts "#$1TypedData_Get_Struct(#$2#{shoes_pat}, &#{shoes_pat}_type, #$3);"
      fo.puts "\#else"
      fo.puts ln
      fo.puts "\#endif"
    else
      fo.puts ln
    end
  end
  mv fn, fpn
  mv fon, fn
end
