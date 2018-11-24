require 'layout/cassowary'

Shoes.app width: 500, height: 400, resizeable: true do
  stack do
    para "Test vfl parser"
    @cls = CassowaryLayout.new()
    @lay = layout use: @cls, width: 450, height: 300 do
      background cornsilk
      para "OverConstrained", name: 'para1'
      edit_line "one", name: 'el1'
      button "two", name: 'but1'
      button "three", name: "but2"
      button "four", name: "but3"
    end
    @lay.start {
      metrics = {
        padding: 80.7
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
        @lay.finish constraints 
     end
    }
 end
  para "After layout"
end
