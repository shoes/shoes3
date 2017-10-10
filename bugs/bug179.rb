Shoes.app width: 600, height: 500 do
  stack do
    para "Quick fun"
    flow do
      button "Button", font: "Menlo Bold 14",width: 200, stroke: red,
          tooltip: "Menlo Bold 14" do
        para "menlo bold button"
      end
      button "icon", width: 80, height: 30, icon: "#{DIR}/static/icon-info.png",
          tooltip: "default right" do
      end
    end
    para "What is normal?"
    flow do
      button "Normal", tooltip: "Normal" do para "normal 1" end
      button "Normal", tooltip: "Arial 12", font: "Arial 12" do para "normal 2" end
    end
    para "try some font names"
    flow do
      button "Curry", font: "Courier New Italic 12",
        tooltip: "Courier New Italic 12" do end
      button "Short", font: "Monaco 17", tooltip: "Monaco 17" do end
    end
    para "testing icon/title interactions"
    flow do
      button "left", width: 80, icon: "#{DIR}/static/icon-info.png",
          titlepos: "left", tooltip: "title left" do
      end
      button "only icon", width: 80, icon: "#{DIR}/static/icon-info.png",
          titlepos: "none", tooltip: "just icon" do
      end
      button "center", width: 80, icon: "#{DIR}/static/icon-info.png",
          titlepos: "center" do
          para "center button pressed"
      end
    end
    para "testing defaults"
    flow do
      button "Fail 1", tooltip: "size missing", font: "Monaco" do end
      button "Fail 2", tooltip: "unknown font", font: "missing font 12" do end
    end
  end
end
