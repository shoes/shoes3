
Shoes.app width: 350, height: 400, resizeable: true do
  stack do
    para "Vfl layout"
    @lay = layout use: :Vfl, width: 300, height: 300 do
      para "OverConstrained", name: 'para1'
      edit_line "one", name: 'el1'
      button "two", name: 'but1'
      button "three", name: "but2"
      button "four", name: "but3"
    end
    @lay.start {
      metrics = {
        'padding' => 10 
      }
      lines = [
        "H:|-[para1(but1)]-[but1]-|",
        "H:|-[el1(but2)]-[but2]-|",
        "H:[but3(but2)]-|",
        "V:|-[para1(el1)]-[el1]-|",
        "V:|-[but1(but2,but3)]-[but2]-[but3]-|"
      ]
      if @lay.vfl_parse lines: lines, metrics: metrics
        constraints = @lay.vfl_constraints #true # true produces hash
        # display purposes only?
        #constraints.each { |c| $stderr.puts c.inspect }
        #@lay.vfl_append  constraints[10]
        @lay.finish constraints 
      end
    }
  end
  #cs = Shoes::Constraint.new 'but1','width', 'eq', 'but3', 'width', 1, 8.0, 1001001000
  #para "#{cs.inspect}"
end
