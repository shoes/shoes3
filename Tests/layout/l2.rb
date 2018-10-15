
class MyLayout 
  attr_accessor :pos_x, :pos_y, :w, :h
  attr_accessor :incr_x, :incr_y, :canvas
  
  def initialize()
    puts "initialized"
    clear
  end
  
  def setup(canvas, attr)
    @canvas = canvas
    @w = attr[:width]
    @h = attr[:height]
    puts "callback: setup #{@w} X #{@h}"
  end
  
  def add(canvas, widget)
    puts "callback add: #{widget.class} #{canvas.contents.size}"
    puts "w: #{widget.width} h: #{widget.height}"
    widget.move @pos_x, @pos_y
    @pos_x += @incr_x
    @pos_y += @incr_y
  end
  
  def clear
    @pos_x = 5
    @pos_y = 5
    @incr_x = 25
    @incr_y = 25
    puts "callback: clear"
  end
  
  def refresh
    @pos_x = 5
    @pos_y = 5
    @canvas.contents.each do |widget| 
      widget.move @pos_x, @pos_y
      @pos_x += @incr_x
      @pos_y += @incr_y
      puts "w: #{widget.width} h: #{widget.height}"
    end
  end
  
end

Shoes.app width: 380, height: 450, resizeable: true do
  stack do
    @p = para "Before layout"
    @ml = MyLayout.new
    @lay = layout manager: @ml, width: 340, height: 380  do
      background yellow
      p1 = para "First Para"
      a = button "one"
      b = button "two"
      p2 = para "I am #{self.class}"
    end
    @p.text = @lay.inspect
    @lay.finish
  end
  @el = edit_line width:40
  @el.text = '-1'
  button "Insert" do
    @lay.insert @el.text.to_i do
      para "inserted #{@el.text}"
    end
  end
  button "delete_at" do
    @lay.delete_at @el.text.to_i do
      para "replaced by deletion"
    end
  end
  button "Clear" do
    @lay.clear { background white }
  end
  button "refresh" do
    @ml.refresh
  end
end
