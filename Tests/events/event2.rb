Shoes.app do  
  stack do
    flow do 
      button "click here" do
        $stderr.puts "button clicked"
      end
    end
  end
  # 2nd window 
  eval IO.read("#{DIR}/samples/simple/chipmunk.rb").force_encoding("UTF-8"), TOPLEVEL_BINDING
  #puts "APPS #{Shoes.APPS}"
  @w2 = Shoes.APPS[1]
  @w2.event = proc do |evt, args|
    $stderr.puts "event handler2 called #{evt} #{args}"
    return true
  end
end
