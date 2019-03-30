#! /usr/bin/env ruby
#  removes #ifdef NEW_MACRO_xxxx
#  example: ./demacro.rb ../shoes/types/svg.c

require 'fileutils'
include FileUtils

argc = ARGV.length
if argc < 1
  puts "no files given"
  exit
end
argc.times do |fi| 
  fn = ARGV[fi]
  puts  "Stripping macros for #{fn}"
  p = File.dirname(fn)
  fon = "#{p}/#{File.basename(fn)}.new"
  fpn = "#{p}/#{File.basename(fn)}.orig"
  fi = File.new(fn)
  fo = File.new(fon, 'w')
  inmacro = false
  copy = true
  fi.each do |ln| 
    if ln[/^\#ifdef NEW_MACRO_/]
      #puts "Copy for #{ln}"
      inmacro = true
      copy = true
      next
    end
    if ln[/^\#else/] && inmacro
      #puts "Skipping"
      copy = false
      next
    end
    if ln[/^\#endif/]  && inmacro
      #puts "Copying after #else-#endif"
      inmacro = false
      copy = true
      next
    end
    if copy
      fo.puts ln
    end
  end
  mv fn, fpn
  mv fon, fn
end
