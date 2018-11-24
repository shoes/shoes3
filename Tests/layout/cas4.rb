require 'layout/cassowary'

Shoes.app width: 350, height: 400, resizeable: true do
  stack do
    para "Test vfl parser"
    @cls = CassowaryLayout.new()
    @lay = layout use: @cls, width: 300, height: 300 do
      background cornsilk
      edit_line "one", name: 'el1'
      button "two", name: 'but1'
    end
    @lay.start {
      metrics = {
        padding: 25
      }
      lines = [
        'V:|-[but1]-[el1]|'
      ]
      @lay.vfl_parse lines: lines, views: @cls.contents, metrics: metrics
      constraints = @lay.vfl_constraints
      @lay.finish constraints 
    }
 end
  para "After layout"
end
