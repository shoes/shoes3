Shoes.setup do
	gem 'cassowary-ruby'
end
require 'cassowary'
include Cassowary
class CassowaryLayout 
  attr_accessor :canvas, :widgets, :solver, :attrs, :left_limit,
	:right_limit, :top_limit, :height_limit, :right_limit_stay,
  :canvas_set, :canvas_w, :canvas_h
  
  def initialize()
    #$stderr.puts "initialized"
    @widgets = {}
    @solver = SimplexSolver.new
    @attrs = {}
    @canvas_set = false
    @canvas_x = 0
    @canvas_y = 0
  end
  
  
  # canvas does not have a width or height. be careful
  def setup(canvas, attr)
    @canvas = canvas
    hgt = 0
    wid = 0
    if (attr && attr[:width]) 
      wid = attr[:width]
    else
      wid = 500
      canvas.style width: wid
    end
    if (attr && attr[:height])
      hgt = attr[:height]
    else
      hgt = 400
      canvas.style height: hgt
    end
		@left_limit = Variable.new(name: 'left', value: 0.0)
		@right_limit = Variable.new(name: 'width', value: wid)
		@top_limit = Variable.new(name: "top", value: 0.0)
		@height_limit = Variable.new(name: "height", value: hgt)
		@solver.add_stay(@left_limit)
		@solver.add_stay(@right_limit, Strength::WeakStrength)
    $stderr.puts "callback: setup #{wid} X #{hgt}"
  end
  
  def add(canvas, widget, attrs)
    # widget is not on-screen and allocated at this time. Pity
    # DO NOT enumerate attrs hash - crash if you try.
    name = attrs && attrs[:name]
    $stderr.puts "callback add: #{name} #{widget.class} #{canvas.contents.size}"
    self.track widget, name if name
  end
  
  def remove(canvas, widget, pos)
    $stderr.puts"callback: remove"
    return true
  end

  
  def track(obj, str)
    @widgets[str] = obj
    vars = {}
    vars['left'] = Variable.new(name: 'left'+str, value: 0)
    vars['width'] = Variable.new(name: 'width'+str, value: 0)
    vars['top'] = Variable.new(name: 'top'+str, value: 0)
    vars['height'] = Variable.new(name: 'height'+str, value: 0)
    @attrs[str] = vars
  end
  
  # parse string like 'b1-width' and return Cassowary Variable
  def var(str)
     str[/(\w+)\-(\w+)/]
     @attrs[$1][$2]
  end
  
  def size(canvas, pass)
    if pass == 0
      return
    else
      $stderr.puts "callback: size Change!  w: #{canvas.width} h:#{canvas.height}"
    end
  end
  
  def widget_defaults
    @canvas.show
		@widgets.each_pair do |id, ele|
			vh = @attrs[id]['height']
      vh.value = ele.height
      vw = @attrs[id]['width']
      vw.value = ele.width
      $stderr.puts "widget #{id} w:#{vw.value} h:#{vh.value}"
		end
  end
 
  def resize(w, h)
    @right_limit.value = w
    @height_limit.value = h
    #@solver.add_stay(@right_limit, Strength::RequiredStrength)
    
    #right_limit_stay = solver.add_constraint(right_limit, strength=REQUIRED)
    #solver.add_constraint(right_limit_stay)
 end
  
  def clear()
    $stderr.puts "callback: clear"
  end
  
  def rules(arg)
    $stderr.puts "callback rules #{arg}"
  end
 
  def finish()
		@widgets.each_pair do |k, widget|
			left = widget.left
			top = widget.top
			height = widget.height
			width = widget.width
			$stderr.puts "From #{left},#{top}"
			left = @attrs[k]['left'].value.to_i
      top = @attrs[k]['top'].value.to_i
			width = @attrs[k]['width'].value.to_i
      height =  @attrs[k]['height'].value.to_i
			$stderr.puts "move to #{left}, #{top} for w:#{width} h:#{height}"
			widget.style width: width
			widget.move(left, top)
		end
  end
  
  # send other methods to @solver
  def method_missing(meth, *args, &block)
    if @solver.respond_to? meth
			@solver.send(meth, *args, &block)
		else
			super
		end
  end
  
end

Shoes.app width: 480, height: 280, resizeable: true do
  stack do
    flow do
      para "Before layout  "
      @p = para "unknown"
    end
    @ml = CassowaryLayout.new
    @lay = layout use: @ml, width: 450, height: 200  do
    #@lay = layout manager: @ml  do
      background teal
      a = button "one", name: 'b1'
      b = button "two", name: 'b2'
      para "Strings Too!", name: 'b3'
    end
    @lay.start do
      # Every thing has a default h and w now. We can compute things
      @ml.rules "foobar" # for a test
      @ml.widget_defaults
      # The two buttons are the same width
      @ml.add_constraint(@ml.var('b1-width').cn_equal @ml.var('b2-width'))
    
      # Button1 starts 50 from the left margin.
      @ml.add_constraint(@ml.var('b1-left').cn_equal @ml.left_limit + 50)
    
      # Button2 ends 50 from the right margin 
      @ml.add_constraint((@ml.left_limit + @ml.right_limit).cn_equal @ml.var('b2-left') + @ml.var('b2-width') + 50)
  
      # Button2 starts at least 50 from the end of Button1. This is the
      # "elastic" constraint in the system that will absorb extra space
      # in the layout.
      @ml.add_constraint(@ml.var('b2-left').cn_equal @ml.var('b1-left') + @ml.var('b1-width') + 50)
  
      # Button1 has a minimum width of 87
      @ml.add_constraint(@ml.var('b1-width').cn_geq 87)
    
      # Button1's preferred width is 87
      @ml.add_constraint(@ml.var('b1-width').cn_equal 87, Strength::StrongStrength)
    
      # Button2's minimum width is 113
      @ml.add_constraint(@ml.var('b2-width').cn_geq 113)
  
      # Button2's preferred width is 113
      @ml.add_constraint(@ml.var('b2-width').cn_equal 113, Strength::StrongStrength)
      
      # string b3 is at the bottom ? 
      @ml.add_constraint(@ml.height_limit.cn_equal @ml.var('b3-top'))
      
      @lay.finish
      @p.text = @lay.inspect    
    end
  end
  button "refresh" do
    @lay.finish
  end
  button "shrink" do
    wid = (@lay.width * 0.9).to_i
    hgt = (@lay.height * 0.9).to_i
    @ml.resize(wid,hgt)
    @lay.style width: wid, height: hgt
    @lay.finish
  end
  button "expand" do
    wid = (@lay.width * 1.1).to_i
    hgt = (@lay.height * 1.1).to_i
    @ml.resize(wid, hgt)
    @lay.style width: wid, height: hgt
    @lay.finish
  end
end
