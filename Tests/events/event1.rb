Shoes.app do
  event do |evt|
    case evt.type
    when :click 
      $stderr.puts "event handler called: #{evt.type} #{evt.button}, #{evt.x} #{evt.y}"
      evt.accept = @ck1.checked?
      if evt.object == @btn
        $stderr.puts "have widget #{evt.object}"
      end
    else
      evt.accept = true
    end
  end  
  
  stack do
    para "Click test 1"
    flow do 
      @ck1 = check checked: true; para "pass clicks to Shoes"
    end
    flow do 
      @btn = button "click here" do
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
