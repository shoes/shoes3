
Shoes.app width: 350, height: 400, resizeable: true do
  stack do
    para "Before layout"
    @lay = layout use: :foobar, width: 300, height: 300 do
      para "First Para", name: 'Label_1'
      button "one", name: 'Button_1'
      button "two", name: 'Button_2'
    end
    @lay.start {
      @lay.rules 'foo.vfl'
      @lay.finish
    }
 end
  para "After layout"
end
