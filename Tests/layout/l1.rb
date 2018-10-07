
class MyLayout 
  attr_accessor :pos_x, :pos_y
  
  def initialize()
    @pos_x = 25
    @pos_y = 25
  end
  
  def add_widget(widget)
    widget.move @pos_x, @pos_y
    @pos_x += 25
    @pos_y += 25
  end
end

Shoes.app width: 350, height: 400, resizeable: true do
  stack do
    para "Before layout"
    @ml = layout manager: MyLayout.new, width: 300, height: 300 do
      background yellow
      p1 = para "First Para"
      a = button "one"
      b = button "two"
      p2 = para "I am #{self}"
    end
  end
  para "After layout"
  para "@ml is #{@ml.inspect}"
end
