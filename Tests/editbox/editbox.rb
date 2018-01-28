Shoes.app height: 400, width: 300 do
  stack do
    flow do
      para "Normal "
       edit_box "Normal", tooltip: "normal"
    end
    flow do 
      para "Font  "
      edit_box "Font Is?", font: "Monaco 9", tooltip: "monaco 9"
    end
    flow do
      para "Stroke "
      edit_box "Stroke", stroke: "green", tooltip: "green"
    end
  end
end
