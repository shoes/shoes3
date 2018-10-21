
class MyLayout 
  attr_accessor :pos_x, :pos_y, :w, :h
  attr_accessor :incr_x, :incr_y, :canvas
  
  def initialize()
    $stderr.puts "initialized"
    self.clear
  end
  
  def setup(canvas, attr)
    @canvas = canvas
    @w = attr[:width]
    @h = attr[:height]
    $stderr.puts "callback: setup #{@w} X #{@h}"
  end
  
  def add(canvas, widget, attrs)
    $stderr.puts "callback add: #{widget.class} #{canvas.contents.size}"
    widget.move @pos_x, @pos_y
    @pos_x += @incr_x
    @pos_y += @incr_y
    name = attrs && attrs[:name]
    if name
      $stderr.puts "Winner, Winner, Chicken Dinner for #{name}"
    end
  end
  
  def remove(canvas, widget, pos)
    $stderr.puts"callback: remove"
    return true
  end
  
  def size(canvas, pass)
    $stderr.puts "callback: size pass=#{pass} w: #{canvas.width} h:#{canvas.height}"
  end
  
  def clear
    @pos_x = 5
    @pos_y = 5
    @incr_x = 25
    @incr_y = 25
    $stderr.puts "callback: clear"
  end

  def finish()
    @canvas.show
  end
  
end

Shoes.app width: 380, height: 450, resizeable: true do
  stack do
    @p = para "Before layout"
    @ml = MyLayout.new
    @lay = layout use: @ml, width: 340, height: 380  do
      background yellow
      p1 = para "First Para", name: 'p1'
      a = button "one", name: 'b1'
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
