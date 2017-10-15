Shoes.app height: 200, width: 300 do
  stack do
    flow do
      para "Normal "
      list_box :items => ["one", "two", "three"], choose: "two"
    end
    flow do 
      para "Font  "
      list_box items: ["Alpha", "Bravo", "Charlie"], font: "Monaco 9",
        choose: "Bravo"
    end
    flow do
      para "Stroke "
      list_box items: ["Ask", "your", "mother!"], choose: "your", stroke: red
    end
  end
end
