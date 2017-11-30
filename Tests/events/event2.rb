Shoes.app do  
  stack do
    tagline "Manage clicks for second Window"
    para "Demonstrates communication with second app"
    flow do
      $ck = check checked: true; para "Pass clicks?"
    end
    flow do 
      button "Set handler for 2nd window" do
        @w2 = Shoes.APPS[-1]
        @w2.event = proc do |evt|
          $stderr.puts "event handler2 with #{evt.type}"
          evt.accept = $ck.checked?
        end
      end
    end
  end

  # 2nd App
  eval IO.read("#{DIR}/samples/simple/chipmunk.rb").force_encoding("UTF-8"), TOPLEVEL_BINDING
  start do
    # move the frontmost app (#2) left and up - it flickers. 
    w = Shoes.APPS[-1]
    x = app.left
    y = app.top
    w.move x+80, y-40
  end
end
