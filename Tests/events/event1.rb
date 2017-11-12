Shoes.app do
  event do |evt,args|
    case evt
    when :click 
      $stderr.puts "event handler called: #{evt} #{args}"
      return false
    else
      return true
    end
  end
  stack do
    flow do 
      button "click here" do
        $stderr.puts "button clicked"
      end
      click do
         puts "flow click"
      end 
    end
  end
  click do
    puts "app click" 
  end
end
