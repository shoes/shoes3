class Sample
  attr_accessor :widgets, :canvas
  
  def initialize()
    @widgets = {}
  end
  
  def setup(canvas, attr)
    @canvas = canvas
  end
  
  def add(canvas, widget, attrs)
    name = attrs && attrs[:name]
    @widgets[name] = widget
  end
  
  def contents
    return @widgets
  end
  
  def remove(canvas, widget, pos)
    return true
  end

  def size(canvas, pass)
  end
  
  def clear()
  end
  
  def finish()
  end
end

Shoes.app width: 350, height: 400, resizeable: true do
  stack do
    para "Test vfl parser"
    @cls = Sample.new()
    @lay = layout use: @cls, width: 300, height: 300 do
      para "OverConstrained", name: 'para1'
      edit_line "one", name: 'el1'
      button "two", name: 'but1'
      button "three", name: "but2"
      button "four", name: "but3"
    end
    @lay.start {
      metrics = {
        el1: 80.7, # what does this mean?
      }
      lines = [
        "H:|-[para1(but1)]-[but1]-|",
        "H:|-[el1(but2)]-[but2]-|",
        "H:[but3(but2)]-|",
        "V:|-[para1(el1)]-[el1]-|",
        "V:|-[but1(but2,but3)]-[but2]-[but3]-|"
      ]
      if @lay.vfl_parse lines: lines, views: @cls.contents, metrics: metrics
        constraints = @lay.vfl_constraints
        # display only!
        constraints.each { |c| $stderr.puts c.inspect }
        @lay.finish constraints 
      end
    }
 end
  para "After layout"
end
