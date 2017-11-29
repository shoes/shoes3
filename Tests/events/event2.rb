Shoes.app do  
  stack do
    flow do 
      button "click here" do
        $stderr.puts "button clicked"
      end
      button "set handler fore 2nd window" do
        @w2 = Shoes.APPS[1]
        @w2.event = proc do |evt|
          $stderr.puts "event handler2 with #{evt.type}"
          evt.accept = false
        end
      end
    end
  end
  # event handler for APPS[0] - starting window
  event do |one|
    puts "First handler clicked"
    one.accept = true
  end
  # 2nd window 
  eval IO.read("#{DIR}/samples/simple/chipmunk.rb").force_encoding("UTF-8"), TOPLEVEL_BINDING
end
