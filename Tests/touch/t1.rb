Shoes.app width: 600, height: 400, menus: false do
  stack do
    button "quit" do
      Shoes.quit
    end
    40.times.each do |i|
     para "Line #{i}"
    end
  end
end
