
Shoes.app width: 350, height: 400, resizeable: true do
  stack do
    para "Before layout"
    @lay = layout use: :Vfl, width: 300, height: 300 do
      para "OverConstrained", name: 'Label1'
      button "one", name: 'Button1'
      button "two", name: 'Button2'
    end
    @lay.start {
      metrics = {}
      lines = [
        "H:|-8-[Button1(==Button2)]-12-[Button2]-8-|",
        "H:|-8-[Label1]-8-|",
        "V:|-8-[Button1,Button2]-12-[Label1(==Button1,Button2)]-8-|"
      ]
      @lay.rules lines: lines, metrics: metrics
      @lay.finish
    }
 end
  para "After layout"
end
