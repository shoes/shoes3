
class MyLayout 
  attr_accessor :pos_x, :pos_y, :w, :h
  attr_accessor :incr_x, :incr_y
  
  def initialize()
    puts "initialized"
    clear
  end
  
  def setup(canvas, attr)
    @w = attr[:width]
    @h = attr[:height]
    puts "callback: setup #{@w} X #{@h}"
  end
  
  def add(canvas, widget)
    puts "callback add: #{widget.inspect} #{canvas.contents.size}"
    @pos_x += @incr_x
    if @pos_x < 0
      @pos_x = 0
      @incr_x = 25
    end
    if @pos_x >= @w 
      @pos_x = @w
      @incr_x = -25
    end
    @pos_y += @incr_y
    if @pos_y <= 0
      @pos_y = 0
      @incr_y = +25
    end
    if @pos_y >= @h
      @pos_y = @h - 25
      @incr_y = -25
    end
    widget.move @pos_x, @pos_y
  end
  
  def clear
    @pos_x = -20
    @pos_y = -20
    @incr_x = 25
    @incr_y = 25
    puts "callback: clear"
  end
  
end

Shoes.app width: 350, height: 450, resizeable: true do
  stack do
    @p = para "Before layout"
    @ml = MyLayout.new
    @lay =layout manager: @ml, width: 340, height: 380  do
      background yellow
      p1 = para "First Para"
      a = button "one"
      b = button "two"
      p2 = para "I am #{self.class}"
    end
    @p.text = @lay.inspect
    @lay.finish
  end
  button "Append" do
    @lay.append { para "appended" }
  end
  button "Clear" do
    @lay.clear { background white }
  end
  button "Prepend" do
    # problem here? 
    @lay.prepend { para "prepended" }
  end
  button "refresh" do
    @lay.refresh
  end
end
