Shoes.app(width: 200, height: 200) do
   @img1 = image "#{DIR}/static/shoes-icon-walkabout.png", left: 0, top: 0, width: 200, height: 200
   @img2 = image "#{DIR}/static/shoes-icon-blue.png", left: 50, top: 50, width: 100, height: 100
   event do |evt|
    $stderr.puts "event called: #{evt.type} at #{evt.x},#{evt.y} mods: #{evt.modifiers}"
    if evt.object 
      $stderr.puts "  for widget: #{evt.object.class} width #{evt.width} height #{evt.height}"
    end
    evt.accept = true
   end
end
