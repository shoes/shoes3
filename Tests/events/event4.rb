class TestWidget < Shoes::Widget
    def initialize()
        self.width = 200; self.height = 200 ## note that you do need this
        flow width: 190, height: 200 do   ###### explicit slot
            background coral
            para "user defined widget"
            click {|btn,x,y,mods| puts "widget clicked btn: #{btn} at #{x},#{y} with #{mods}"}
        end 
    end
end

Shoes.app do  
  stack do
    tagline "Clicks for non-native widgets"
    flow do
      $ck = check checked: true; para "Pass clicks?"
    end
    flow do
      @img = image "#{DIR}/static/shoes-icon-walkabout.png", width: 200, height: 200
      @img.click {|btn,x,y,mods|  @eb.append "Image clicked btn: #{btn} at #{x},#{y} with #{mods}\n"}
      @svg = svg "#{DIR}/samples/good/paris.svg", width: 200, height: 200, group: "#diamond_4"
      @svg.click {|btn,x,y,mods|  @eb.append "Svg clicked btn: #{btn} at #{x},#{y} with #{mods}\n"}
      @plt = plot 200, 200, title: "Test plot"
      @plt.click {|btn,x,y,mods|  @eb.append "Plot clicked btn: #{btn} at #{x},#{y} with #{mods}\n"}
      @tw = test_widget
    end
    @eb = edit_box width: 500, height: 200
  end
  event do |evt| 
    puts "event called #{evt.type} at #{evt.x},#{evt.y} mods: #{evt.modifiers}"
    evt.accept = $ck.checked? 
  end
end
