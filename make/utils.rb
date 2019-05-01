# Call this to switch RUBY_VERSION constant to what's inside the
# rbconfig.rb give in the argument.
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
  Object.instance_eval{ remove_const :RUBY_VERSION }
  Object.const_set("RUBY_VERSION", rbv)
  puts "Using Ruby version  #{RUBY_VERSION}"
end
