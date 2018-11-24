# Tests/layout/l4.rb - grid layout

class GridLayout

  attr_accessor :ncol, :nrow, :colsz, :rowsz, :widgets, :hpad, :vpad

  def initialize(attr = {})
    @ncol = 0
    @nrow = 0
    @colsz = 0
    @rowsz = 0
    @widgets = []
    @hpad = attr[:hpad] ? attr[:hpad] : 1
    @vpad = attr[:vpad] ? attr[:vpad] : 1
  end
  
  def setup(canvas, attr)
  end

  def add(canvas, ele, attr)
    col = attr[:col] ? attr[:col]-1 : 0
    row = attr[:row] ? attr[:row]-1 : 0
    rspan = attr[:rspan] 
    cspan = attr[:cspan] 
    @ncol = [@ncol, col + (cspan ? cspan : 1)].max
    @nrow = [@nrow, row + (rspan ? rspan : 1)].max
    widgets << {ele: ele, col: col, row: row, cspan: cspan, rspan: rspan}
   end
  
  def remove
  end
  
  def clear
  end
    
  def size (canvas, pass)
    return if pass == 0
    @rowsz = (canvas.height / @nrow).to_i
    @colsz = (canvas.width / @ncol).to_i
    @widgets.each do |entry| 
      x = entry[:col] * @colsz + @hpad
      y = entry[:row] * @rowsz + @vpad
      widget = entry[:ele]
      widget.move(x, y)
      if entry[:cspan]
        w = entry[:cspan] * @colsz - @hpad
        if widget.width != w 
          widget.style width: w
        end
      end
      if entry[:rspan]
        h = entry[:rspan] * @rowsz - @vpad
        if widget.height != h
          widget.style height: h
        end
      end
    end
  end
  
  def finish
  end
      
end  

Shoes.app width: 400, height: 400 do 
  stack do
    para "Before Grid"
    grid = GridLayout.new  vpad: 5, hpad: 3
    @lay = layout use: grid, width: 300, height: 305 do
      background aliceblue
      button "one", col: 1, row: 1, cspan: 2, rspan: 1 
      button "two", col: 3, row: 1, cspan: 2, rspan: 2
      para "Long String of characters", col: 2, row: 5, cspan: 2
      button "three", col: 2, row: 3, cspan: 1
      svg "#{DIR}/samples/good/paris.svg", width: 10, height: 10, col: 3, row: 3, rspan: 2,
        cspan: 3
      image "http://shoesrb.com/img/shoes-icon.png", row: 5, col: 4, cspan: 2, rspan: 2
      edit_line "foobar", row: 7, col: 1, cspan: 2
    end
    flow do
      button "+" do
        @lay.style width: (@lay.width * 1.1).to_i
      end
      button "-" do
        @lay.style width: (@lay.width * 0.9).to_i
      end
    end
  end
end
