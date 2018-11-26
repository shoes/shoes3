
require 'layout/grid'

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
