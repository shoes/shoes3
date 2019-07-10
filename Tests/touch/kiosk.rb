# example inspired by 
# https://startingelectronics.org/projects/raspberry-PI-projects/kiosk-gtk-glade/
# issue #447
$shops = [
    { name: "Joes Shoes", picture: "res/shop1.png", point: [104, 92]},
    { name: "Coffee Shop", picture: "res/shop2.png", point: [485, 92]},
    { name: "Post Office", picture: "res/shop3.png", point: [104, 307]},
    { name: "Meg's Clothing", picture: "res/shop4.png", point: [485,307]}
  ]

Shoes.app width: 800, height: 400 do
  background lightpink
  
  # returns a layout with buttons that change the panel
  def mall_view 
    fl = flow do
      stk = stack width: 0.2 , margin: 8 do
        $shops.each_index do |index| 
          button "#{$shops[index][:name]}", height:80, width: 120, margin: 8 do
            @panel.clear do 
              second_panel(index)
            end
          end
        end
      end
      @mall_stk = stack width: 0.8, height: 400 do
        @mall_img = image "res/mall.svg.png"
      end
    end
   return fl
  end
  
  # return a flow 
  def second_panel(index)
    #hsh = $shops[index]
    #alert "calling #{hsh[:name]} panel"
    fl =flow do
      stack width: 0.2, margin: 8 do
        button 'Back', height: 380, width: 120, margin: 8 do 
          @panel.clear { mall_view }
        end
      end
      stack width: 0.8 ,height: 400 do
        image $shops[index][:picture]
      end
    end
    return fl
  end
  
  @panel = stack do
    mall_view
    click do |btn,x,y,mods|
      idx = hit_detect(x,y)
      if idx 
        @panel.clear do
          second_panel idx
        end
      end
    end
  end
  
  def hit_detect(x, y)
    rx = x - @mall_stk.left
    ry = y - @mall_stk.top
    if rx > 0 && ry > 0 && rx < @mall_stk.width && ry < @mall_stk.height
      $shops.each_index do |idx|
        hsh = $shops[idx]
        nm = hsh[:name]
        px = hsh[:point][0]
        py = hsh[:point][1]
        #puts "checking #{rx},#{ry} against #{px}, #{py}"
        xr = (rx > px - 30) && (rx < px + 30)
        yr = (ry > py - 30) && (ry < py + 30)
        if (xr && yr)
          return idx
        end
      end
    end
    return nil
  end
end
