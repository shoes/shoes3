
class MyLayout 
  attr_accessor :pos_x, :pos_y, :w, :h
  attr_accessor :contents, :canvas
  
  def initialize()
    @pos_x = 25
    @pos_y = 25
    @contents = []
  end
  
  def setup(canvas, attr)
    @canvas = canvas
    @w = attr[:width]
    @h = attr[:height]
    puts "setup #{@w} X #{@h}"
  end
  
  def add(canvas, widget, attrs)
    @contents << widget
    widget.move @pos_x, @pos_y
    @pos_x += 25
    @pos_y += 25
  end
  
  def remove(canvas, widget, pos)
    return true
  end

  def size(canvas, pass)
    $stderr.puts "callback: size pass=#{pass}"
  end
  
  def clear()
  end
  
  def finish()
    @canvas.show
  end
end

Shoes.app width: 350, height: 400, resizeable: true do
  stack do
    para "Before layout"
    @ml = MyLayout.new
    @lay = layout use: @ml, width: 300, height: 300 do
      background yellow
      p1 = para "First Para"
      a = button "one"
      b = button "two"
      p2 = para "I am #{self.class}"
    end
  end
  @lay.start {@lay.finish}
  para "After layout"
  para "@ml is #{@ml.class}"
end
