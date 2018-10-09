
class MyLayout 
  attr_accessor :pos_x, :pos_y, :w, :h
  attr_accessor :contents
  
  def initialize()
    @pos_x = 25
    @pos_y = 25
    @contents = []
  end
  
  def setup(attr)
    @w = attr[:width]
    @h = attr[:height]
    puts "setup #{@w} X #{@h}"
  end
  
  def add(widget)
    @contents << widget
    widget.move @pos_x, @pos_y
    @pos_x += 25
    @pos_y += 25
  end
end

Shoes.app width: 350, height: 400, resizeable: true do
  stack do
    para "Before layout"
    @ml = MyLayout.new
    layout manager: @ml, width: 300, height: 300 do
      background yellow
      p1 = para "First Para"
      a = button "one"
      b = button "two"
      p2 = para "I am #{self.class}"
    end
  end
  para "After layout"
  para "@ml is #{@ml.class}"
end
