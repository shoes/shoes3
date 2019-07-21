Shoes.app width: 600, height: 400, menus: false do
  stack do
    button "quit" do
      Shoes.quit
    end
    10.times.each do |i|
     para "Line #{i+1}"
    end
    @eb = edit_box "First"
    30.times.each do |i|
      @eb.append "\nline #{i+1}"
    end
    30.times.each do |i|
      para "Line #{i+11}"
    end
  end
end
