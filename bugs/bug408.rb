Shoes.app fullscreen: true do
  para "press 'q' to exit anything else to shrink"
  keypress do |k| 
    if k == 'q' || k =='Q'
      Shoes.quit 
    else
      app.fullscreen = false
    end
  end
end
