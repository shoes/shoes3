# grid layout

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
