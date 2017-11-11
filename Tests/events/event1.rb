Shoes.app do
  event do |evt,args|
    $stderr.puts "event handler called #{evt} #{args}"
    false
  end
  stack do
    flow do 
      button "click here" do
        $stderr.puts "button clicked"
      end
    end
  end
  #app.events = true
end
