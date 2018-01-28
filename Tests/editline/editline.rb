Shoes.app height: 200, width: 300 do
  stack do
    flow do
      para "Normal "
       edit_line "Normal", tooltip: "Normal"
    end
    flow do 
      para "Font  "
      edit_line "Font Is?", font: "Monaco 9", tooltip: "monaco 9"
    end
    flow do
      para "Stroke "
      edit_line "Stroke", stroke: green, tooltip: "green"
    end
  end
end
