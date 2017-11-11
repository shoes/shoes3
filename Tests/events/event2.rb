Shoes.app do  
  stack do
    flow do 
      button "click here" do
        $stderr.puts "button clicked"
      end
    end
  end
  app.event = proc do |evt, args|
    $stderr.puts "event handler2 called #{evt} #{args}"
    true
  end
end
